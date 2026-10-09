// win-client-event-api.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "UI.h"
#include "CfgReader.h"

int main()
{
	try {
		std::string url;
		std::string apiKey;

		getServerConfig(url, apiKey);

		std::cout << "URL: " << url << std::endl;

		UI ui(url, apiKey);
		ui.start();
	}
	catch (const std::exception& e) {
		std::cerr << "Fatal error:" << e.what() << std::endl;
	}
	return 0;
}

