#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <string>
#include <memory>
#include <windows.h>

#include "CfgReader.h"

std::wstring toUTF8(const std::string& str);

struct HttpResponse {
	DWORD status;
	std::string body;
};

class WinHttpClient{
public:
	WinHttpClient(const std::string& url, const std::string& apiKey);
	~WinHttpClient() = default;

	void setUrl(const std::string& url);
	void setApiKey(const std::string& apiKey);

	void setTimeOut() {};
	void setTimeOut(int seconds);

	HttpResponse get(const std::string &target);
	HttpResponse post(const std::string& target, const std::string& body, const std::string& content_type, const std::string* boundary = nullptr);


private:
	HttpResponse perform(const std::string& command,
				 const std::string& target,
				 const std::string* body,
				 const std::string& content_type,
				 const std::string* boundary);

	struct HandleDeleter {
		void operator()(void* handle) const noexcept;
	};

	using Handle = std::unique_ptr<void, HandleDeleter>;

	Handle _session;
	std::string _apiKey;
	std::string _url;
	nlohmann::json _config;
};
