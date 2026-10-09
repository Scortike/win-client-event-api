#include "CfgReader.h"

#include <fstream>
#include <Windows.h>


void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return;

	std::size_t pos = 0;

	while ((pos = str.find(from, pos)) != std::string::npos)
	{
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
}

void getServerConfig(std::string& url, std::string& apiKey, const std::string& configRelativePath) {

	nlohmann::json config;
	getConfig(config, configRelativePath);

	url = config["url"].get<std::string>();
	apiKey = config["api_key"].get<std::string>();
}

void getConfig(nlohmann::json& config, const std::string& configRelativePath) {
	std::string configFilePath;
	getConfigFilePath(configFilePath, configRelativePath);
	std::ifstream configFile(configFilePath);	
	if (!configFile) {
		throw std::runtime_error("Failed to open config file: " + configFilePath);
	}
	configFile >> config;
}

void getExecutablePath(std::string& executablePath) {
	char buffer[MAX_PATH];
	DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
	if (length == 0 || length == MAX_PATH) {
		throw std::runtime_error("Failed to get executable path");
	}
	executablePath = std::string(buffer, length);
}

void getConfigFilePath(std::string& configFilePath, const std::string& configRelativePath) {
	std::string executablePath;
	getExecutablePath(executablePath);
	std::filesystem::path exePath(executablePath);
	std::filesystem::path configPath = exePath.parent_path() / configRelativePath;
	configFilePath = configPath.string();
}