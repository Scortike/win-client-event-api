#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <string>
#include <memory>


class WinHttpClient{
public:
	WinHttpClient(const std::string& url, const std::string& apiKey);
	~WinHttpClient() = default;

	void setUrl(const std::string& url);
	void setApiKey(const std::string& apiKey);
private:

	struct HandleDeleter {
		void operator()(void* handle) const noexcept;
	};

	using Handle = std::unique_ptr<void, HandleDeleter>;

	Handle _session;
	std::string _apiKey;
	std::string _url;
};
