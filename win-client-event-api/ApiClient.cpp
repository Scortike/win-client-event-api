#include "ApiClient.h"

#include <rpc.h>
#include <fstream>
#include <iterator>
#pragma comment(lib, "Rpcrt4.lib")


ApiClient::ApiClient(const std::string& url, const std::string& apiKey)
	: _httpClient(url, apiKey), _url(url), _apiKey(apiKey) {
	getConfig(_config, API_CFG_FILE_PATH);
}

void ApiClient::setUrl(const std::string& url) {
	_url = url;
	_httpClient.setUrl(url);
}

void ApiClient::setApiKey(const std::string& apiKey) {
	_apiKey = apiKey;
	_httpClient.setApiKey(apiKey);
}	

HttpResponse ApiClient::getStatus() {
	std::string target = _config["target"].get<std::string>() + _config["end_points"]["status"].get<std::string>();
	HttpResponse response = _httpClient.get(target);
	return response;
}

HttpResponse ApiClient::getEventsByFilter(const std::string& filter) {
	std::string target = _config["target"].get<std::string>() 
		+ _config["end_points"]["event"].get<std::string>()
		+ "?" + filter;
	HttpResponse response = _httpClient.get(target);
	return response;
}

HttpResponse ApiClient::getEvents() {
	std::string target = _config["target"].get<std::string>() + _config["end_points"]["event"].get<std::string>();
	HttpResponse response = _httpClient.get(target);
	return response;
}

HttpResponse ApiClient::postEvent(const std::string& fileName) {
	std::string timestamp = getTimestamp();
	std::string uuid = getUUID();
	nlohmann::json event = _config["patterns"].at("post_event");

	event["timestamp"] = timestamp;
	event["event_uid"] = uuid;
	event["event_code"] = 1 + rand() % 10;
	event["file_name"] = fileName;

	std::string body = event.dump();

	std::string target = _config["target"].get<std::string>() + _config["end_points"]["event"].get<std::string>();
	std::string type = _config["content_type"]["json"].get<std::string>();

	HttpResponse response = _httpClient.post(target, body, type);
	return response;
}

HttpResponse ApiClient::postImage(const std::string& eventId, const std::string& filePath, const std::string& filename) {
	std::string target = _config["target"].get<std::string>() + _config["end_points"]["image"].get<std::string>();
	std::string contentType = _config["content_type"]["image"].get<std::string>();
	replaceAll(target, "{event_id}", eventId);
	
	std::string boundary = "----Boundary";
	const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	const size_t max_index = (sizeof(charset) - 1);
	for (int i = 0; i < 16; ++i) {
		boundary += charset[rand() % max_index];
	}

	std::string binaryData = readFile(filePath);

	std::string body;
	body += "--" + boundary + "\r\n";
	std::string contentdispation = _config["patterns"]["post_image"].get<std::string>();
	replaceAll(contentdispation, "{filename}", filename);
	replaceAll(contentdispation, "{content_type}", contentType);
	body += contentdispation + "\r\n\r\n";
	body += binaryData;
	body += "\r\n--" + boundary + "--\r\n";

	HttpResponse response = _httpClient.post(target, body, contentType, &boundary);
	return response;
}

std::string ApiClient::getTimestamp()
{
	const auto now = std::chrono::system_clock::now();
	const std::time_t time = std::chrono::system_clock::to_time_t(now);

	std::tm utcTime{};

	gmtime_s(&utcTime, &time);

	std::ostringstream oss;
	oss << std::put_time(&utcTime, "%Y-%m-%dT%H:%M:%SZ");

	return oss.str();
}

std::string ApiClient::getUUID()
{
	UUID uuid;
	UuidCreate(&uuid);
	RPC_CSTR uuidStr;
	UuidToStringA(&uuid, &uuidStr);
	std::string result(reinterpret_cast<char*>(uuidStr));
	RpcStringFreeA(&uuidStr);
	return result;
}

std::string ApiClient::readFile(const std::string& path) {
	std::ifstream in(path, std::ios::binary);
	if (!in) throw std::runtime_error("Cannot open file: " + path);
	return std::string(std::istreambuf_iterator<char>(in),
		std::istreambuf_iterator<char>());
}
