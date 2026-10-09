#include "HttpClient.h"

#include <stdexcept>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")


std::wstring toUTF8(const std::string& str) {
	if(str.empty()) {
		return std::wstring();
	}

	int utf8Len = MultiByteToWideChar(CP_UTF8, 
									  MB_ERR_INVALID_CHARS,
									  str.data(), 
									  static_cast<int>(str.size()), 
									  NULL, 0);
	if (utf8Len <= 0) {
		throw std::runtime_error("Failed to convert string to UTF-16");
	}
	std::wstring wideStr(static_cast<size_t>(utf8Len), L'\0');
	MultiByteToWideChar(CP_UTF8, 0,
						str.data(),
						static_cast<int>(str.size()),
						&wideStr[0], 
						utf8Len);
	return wideStr;
}

WinHttpClient::WinHttpClient(const std::string& url, const std::string& apiKey)
	: _url(url), _apiKey(apiKey) {

	_session.reset(WinHttpOpen(
		L"Chernetskyi Client",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0
	));

	setTimeOut(5);

	if (!_session) {
		throw std::runtime_error("Failed to open WinHTTP session");
	}

	getConfig(_config, REQUEST_CFG_FILE_PATH);
}

void WinHttpClient::setUrl(const std::string& url) {
	_url = url;
}

void WinHttpClient::setApiKey(const std::string& apiKey) {
	_apiKey = apiKey;
}

void WinHttpClient::HandleDeleter::operator()(void* handle) const noexcept {
	if (handle) {
		WinHttpCloseHandle(handle);
	}
}

void WinHttpClient::setTimeOut(int seconds) {
	int ms = seconds * 1000;
	WinHttpSetTimeouts(_session.get(), seconds, seconds, seconds, seconds);
}

HttpResponse WinHttpClient::get(const std::string &target) {
	HttpResponse response = execWithRetry([&]()
		{
			return perform("GET", target, nullptr, std::string(), nullptr);
		});
	return response;
}

HttpResponse WinHttpClient::post(const std::string& target, const std::string& body, const std::string& content_type, const std::string* boundary) {
	HttpResponse response = execWithRetry([&]()
		{
			return perform("POST", target, &body, content_type, boundary);
		});
	return response;
	
}

HttpResponse WinHttpClient::perform(const std::string& command,
							const std::string& target, 
							const std::string* body,
							const std::string& content_type,
							const std::string* boundary) {
	// connection handle
	Handle connection(WinHttpConnect(_session.get(),
									 toUTF8(_url).c_str(),
									 INTERNET_DEFAULT_HTTPS_PORT, 0));
	if (!connection) {
		throw std::runtime_error("Failed to connect to server");
	}

	// open request handle
	Handle request(WinHttpOpenRequest(connection.get(),
									  toUTF8(command).c_str(), 
									  toUTF8(target).c_str(), 
									  NULL, 
									  WINHTTP_NO_REFERER, 
									  WINHTTP_DEFAULT_ACCEPT_TYPES, 
									  WINHTTP_FLAG_SECURE));
	if (!request) {
		throw std::runtime_error("Failed to open HTTP request");
	}


	std::wstring headers;
	if (!_apiKey.empty()) {
		std::string apiKeyHeader = _config["flag"]["api_key"].get<std::string>();
		replaceAll(apiKeyHeader, "{api_key}", _apiKey);
		headers += toUTF8(apiKeyHeader) + L"\r\n";
	}
	if (body) {
		std::string contentTypeHeader = _config["flag"]["content_type"].get<std::string>();
		replaceAll(contentTypeHeader, "{content_type}", content_type);
		headers += toUTF8(contentTypeHeader);
		if (boundary) {
			std::string baoundaryHeader = _config["flag"]["boundary"].get<std::string>();
			replaceAll(baoundaryHeader, "{boundary}", *boundary);
			headers += toUTF8(baoundaryHeader);
		}
		headers += L"\r\n";
	}
	LPCWSTR headerPtr = WINHTTP_NO_ADDITIONAL_HEADERS;
	DWORD headerLen = 0;
	if (!headers.empty()) {
		headerPtr = headers.c_str();
		headerLen = static_cast<DWORD>(headers.size());
	}

	LPVOID data = WINHTTP_NO_REQUEST_DATA;
	DWORD dataLen = 0;
	if (body) {
		data = const_cast<char*>(body->c_str());
		dataLen = static_cast<DWORD>(body->size());
	}


	if (!WinHttpSendRequest(request.get(), headerPtr, headerLen, data, dataLen, dataLen, 0)) {
		throw std::runtime_error("Failed to send HTTP request");
	}

	if (!WinHttpReceiveResponse(request.get(), nullptr)) {
		throw std::runtime_error("Failed to receive HTTP response");
	}

	HttpResponse response;

	DWORD statusSize = sizeof(response.status);
	if (!WinHttpQueryHeaders(request.get(),
							 WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
							 WINHTTP_HEADER_NAME_BY_INDEX, &response.status, &statusSize,
							 WINHTTP_NO_HEADER_INDEX)) {
		throw std::runtime_error("Failed to query HTTP headers");
	}	

	DWORD bufSize = 0;
	do
	{
		if (!WinHttpQueryDataAvailable(request.get(), &bufSize)) {
			break;
		}
		
		if (bufSize == 0) {
			break;
		}

		std::vector<char> buffer(bufSize);
		DWORD bytesRead = 0;

		if(!WinHttpReadData(request.get(), buffer.data(), 
							bufSize, &bytesRead)) {
			break;
		}

		response.body.append(buffer.data(), bytesRead);
	} while (bufSize > 0);

	return response;
}

HttpStatusCategory WinHttpClient::classifyStatus(const DWORD& statuscode) {
	if (statuscode >= 200 && statuscode < 300)
		return HttpStatusCategory::Success;
	if (statuscode >= 400 && statuscode < 500)
		return HttpStatusCategory::ClientError;
	if (statuscode >= 500 && statuscode < 600)
		return HttpStatusCategory::ServerError;

	return HttpStatusCategory::Unexpected;
}

std::string WinHttpClient::responceHandle(const HttpResponse& res) {
	std::stringstream ss;

	if (classifyStatus(res.status) == HttpStatusCategory::Success) {
		ss << "Success!" << std::endl;
		ss << "CODE: " << res.status << std::endl;
		nlohmann::json body = nlohmann::json::parse(res.body);
		ss << body.dump(4) << std::endl;
	}
	else {
		ss << "ERORR!!!" << std::endl;
		nlohmann::json body = nlohmann::json::parse(res.body);
		ss << "CODE: " << res.status << " " << body["error"]["code"].get<std::string>() << std::endl;
		ss << "Message: " << body["error"]["message"].get<std::string>() << std::endl;
		ss << body["error"]["details"].dump(4);

	}
	return ss.str();

}
