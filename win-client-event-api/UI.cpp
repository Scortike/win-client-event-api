#include "UI.h"


#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>

UI::UI(std::string url, std::string key) : 
	_client(url, key), _url(url), _key(key)
{ }

void UI::start()
{
	//checkConnection();
	mainMenu();
}

void UI::showMenu() {
	std::cout << "Select an option:" << std::endl;
	std::cout << "\t 1. Check the connection" << std::endl;
	std::cout << "\t 2. Get a list of events" << std::endl;
	std::cout << "\t 3. Send a new event" << std::endl;
	std::cout << "\t 4. Get the events with no images" << std::endl;
	std::cout << "\t 0. Exit" << std::endl << std::endl;
}

void UI::mainMenu() {
	int choose = -1;
	while (choose != 0) {
		showMenu();
		if (!(std::cin >> choose))
		{
			std::cin.clear();
			std::cin.ignore(1000, '\n');

			std::cout << "Invalid input! Enter a number" << std::endl;
			choose = -1;
			continue;
		}

		switch (choose) {
		case 1:
			checkConnection();
			break;
		case 2:
			eventList();
			break;
		case 3:
			eventSetUp();
			break;
		case 4:
			eventlistWithoutImg();
		default: 
			std::cout << "Invalid input! Enter a number (0 - 4)" << std::endl;
			break;
		}
		

	}
}

void UI::checkConnection() {
	std::cout << "Get a status" << std::endl;
	HttpResponse res = _client.getStatus();
}

void UI::eventList() {
	std::cout << "Get a list of events" << std::endl;
	HttpResponse res = _client.getEvents();
}

void UI::eventSetUp() {
	std::cout << "Prepare event..." << std::endl;
	std::string filePath = filePathDialog();
	HttpResponse res = _client.postEvent(std::filesystem::path(filePath).filename().string());

	if (res.status == 201) {
		int choose = -1;
		int event_id = nlohmann::json::parse(res.body)["event_id"].get<int>();
		auto chooseDlg = [&]() {
			std::cout << "Upload File?:" << std::endl;
			std::cout << "\t 1. Yes." << std::endl;
			std::cout << "\t 2. Later (Back)" << std::endl;
			};
		while (choose == -1) {
			chooseDlg();

			if (!(std::cin >> choose))
			{
				std::cin.clear();
				std::cin.ignore(1000, '\n');

				std::cout << "Invalid input! Enter a number" << std::endl;
				choose = -1;
				continue;
			}

			switch (choose) {
			case 1:
				sendImage(std::to_string(event_id), filePath);
			}
		}
	}

}

void UI::eventlistWithoutImg() {
	std::cout << "Request list..." << std::endl;
	int choose = -1;
	HttpResponse res = _client.getEventsByFilter();
	auto chooseDlg = [&]() {
		std::cout << "Wonna upload missing File?:" << std::endl;
		std::cout << "\t 1. Yes." << std::endl;
		std::cout << "\t 2. Later (Back)" << std::endl;
		};
	while (choose == -1) {
		chooseDlg();

		if (!(std::cin >> choose))
		{
			std::cin.clear();
			std::cin.ignore(1000, '\n');

			std::cout << "Invalid input! Enter a number" << std::endl;
			choose = -1;
			continue;
		}

		if (choose == 2) return;
		
	}
	std::vector<nlohmann::json> events = nlohmann::json::parse(res.body)["items"].get<std::vector<nlohmann::json>>();
	auto chooseEvent = [&](std::vector<nlohmann::json> events) {
		int counter = 1;
		for (auto event : events) {
			std::cout << "\t" << counter << ". event_uid = " << event["event_id"] << ", file_name = " << event["file_name"] << std::endl;
			counter++;
		}
	};
	while (choose != 0) {
		std::cout << "Which event?" << std::endl;
		chooseEvent(events);
		std::cout << "\t0. Back" << std::endl;
		if (!(std::cin >> choose))
		{
			std::cin.clear();
			std::cin.ignore(1000, '\n');

			std::cout << "Invalid input! Enter a number" << std::endl;
			choose = -1;
			continue;
		}

		if (choose == 0) return;

		if (choose < 0 || choose > events.size()) {
			std::cout << "Invalid input! Enter a number 0 <= n <= "<< events.size() << std::endl;
			choose = -1;
			continue;
		}
		std::string filePath;
		bool filePathFlag = false;
		do {
			filePath = filePathDialog();
			std::string local_name = std::filesystem::path(filePath).filename().string();
			std::string server_name = events[choose - 1]["file_name"].get<std::string>();
			filePathFlag = (local_name == server_name);
		} while (!filePathFlag);
		sendImage(std::to_string(events[choose - 1]["event_id"].get<int>()), filePath);
		choose = 0;
	}
}

void UI::sendImage(const std::string& eventId, const std::string& filePath) {
	std::cout << "Sending file..." << std::endl;
	HttpResponse res = _client.postImage(eventId, filePath, std::filesystem::path(filePath).filename().string());
}

std::string UI::filePathDialog() {
	bool filePathFlag = false;
	std::string filePath;
	while (!filePathFlag)
	{
		std::cout << "Choose file (.jpg/.png and <= 5 Mib)" << std::endl;
		if (!(std::cin >> filePath))
		{
			std::cin.clear();
			std::cin.ignore(1000, '\n');

			std::cout << "Invalid input! Enter a path" << std::endl;
			continue;
		}
		if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath)) {
			std::cout << "Cannot open file! Provide correct path";
			continue;
		}
		std::string extension = std::filesystem::path(filePath).extension().string();
		if (!(extension == ".jpg" || extension == ".png")) {
			std::cout << "The file must be a PNG or JPG, yours - " << extension << std::endl;
			continue;
		}
		constexpr std::uintmax_t MAX_SIZE = 5ULL * 1024 * 1024;
		if (std::filesystem::file_size(filePath) > MAX_SIZE) {
			std::cout << "File too big, MAX - 5 Mib";
			continue;
		}

		filePathFlag = true;
	}
	return filePath;
}
