#pragma once

#include <string>
#include "HttpClient.h"
#include <nlohmann/json.hpp>

class ApiClient {
public:
	ApiClient(const std::string& url, const std::string& apiKey);
	~ApiClient() = default;

	void setUrl(const std::string& url);
	void setApiKey(const std::string& apiKey);

	HttpResponse getStatus();
	HttpResponse getEvents();
	HttpResponse getEvents(const std::string& filter);
	HttpResponse postEvent(const int& eventCode, const std::string& fileName);
	HttpResponse postImage(const std::string& eventId, const std::string& filePath, const std::string& filename);

	std::string getTimestamp();
	std::string getUUID();
	std::string readFile(const std::string& path);


private:
	WinHttpClient _httpClient;
	std::string _url;
	std::string _apiKey;
	nlohmann::json _config;
};