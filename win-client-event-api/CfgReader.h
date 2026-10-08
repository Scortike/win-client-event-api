#include <string>

#include "defines.h"


void getServerConfig(std::string& url, std::string& apiKey, const std::string& configRelativePath = CFG_FILE_PATH);

void getExecutablePath(std::string& executablePath);

void getConfigFilePath(std::string& configFilePath, const std::string& configRelativePath = CFG_FILE_PATH);