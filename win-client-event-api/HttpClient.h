#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <string>
#include <memory>
#include <windows.h>
#include <winhttp.h>

class WinHttpClient{
public:
	WinHttpClient(const std::string& url, const std::string& apiKey) : _url(url), _apiKey(apiKey) {}
	~WinHttpClient() = default;

	void setUrl(const std::string& url);
	void setApiKey(const std::string& apiKey);
private:
	struct HandleDeleter { void operator()(HINTERNET handle) const noexcept; };

	using Handle = std::unique_ptr<void, HandleDeleter>;

	Handle _session;
	std::string _apiKey;
	std::string _url;
};
