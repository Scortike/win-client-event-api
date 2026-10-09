// win-client-event-api.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "UI.h"
#include "CfgReader.h"

int main()
{
	std::string url;
	std::string apiKey;

	getServerConfig(url, apiKey);

	std::cout << "URL: " << url << std::endl;
	std::cout << "API Key: " << apiKey << std::endl;

	UI ui(url, apiKey);
	ui.start();

	return 0;
}

