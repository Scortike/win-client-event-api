#include "HttpClient.h"

#include <stdexcept>

WinHttpClient::WinHttpClient(const std::string& url, const std::string& apiKey)
	: _url(url), _apiKey(apiKey) {
	// Initialize WinHTTP session
	_session.reset(WinHttpOpen(
		L"Chernetskyi Client",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 
		WINHTTP_NO_PROXY_NAME, 
		WINHTTP_NO_PROXY_BYPASS, 
		0
	));
	if (!_session) {
		throw std::runtime_error("Failed to open WinHTTP session");
	}
}

void WinHttpClient::setUrl(const std::string& url) {
	_url = url;
}

void WinHttpClient::setApiKey(const std::string& apiKey) {
	_apiKey = apiKey;
}