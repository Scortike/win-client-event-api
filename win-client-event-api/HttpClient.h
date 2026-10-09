#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <string>
#include <memory>
#include <windows.h>
#include <thread>
#include <iostream>

#include "CfgReader.h"

std::wstring toUTF8(const std::string& str);

enum class HttpStatusCategory
{
	Success,
	ClientError,
	ServerError,
	Unexpected
};

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

	void setTimeOut(int seconds);

	HttpResponse get(const std::string &target);
	HttpResponse post(const std::string& target, const std::string& body, const std::string& content_type, const std::string* boundary = nullptr);

	template <typename Operation>
	HttpResponse execWithRetry(Operation&& operation, int maxRetry = 4) {
		for (int attempt = 0;; ++attempt)
		{
			HttpResponse response = operation();
			std::cout << responseHandle(response);

			const bool retryable = response.status == 429 || response.status == 502 ||
								   response.status == 503 || response.status == 504;

			if (!retryable || attempt >= maxRetry)
			{
				return response;
			}
			int delay_s = 1 << attempt;

			std::cout << "Retry send request in " << delay_s << " secs." << std::endl;
			
			const auto delay = std::chrono::seconds(delay_s);
			std::this_thread::sleep_for(delay);
		}
	}

	std::string responseHandle(const HttpResponse& res);

	HttpStatusCategory classifyStatus(const DWORD& statusCode);


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
