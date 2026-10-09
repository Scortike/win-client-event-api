#pragma once

#include <string>
#include "ApiClient.h"

class UI {
public:
	UI(std::string url, std::string key);
	~UI() = default;

	void start();

	void showMenu();
	void mainMenu();

	void checkConnection();
	void eventList();
	void eventlistWithoutImg();
	void eventSetUp();
	void sendImage(const std::string& event_id, const std::string& file_path);

	std::string filePathDialog();

private:
	ApiClient _client;
	std::string _url;
	std::string _key;
	std::string _menu;

};