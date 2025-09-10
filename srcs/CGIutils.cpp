#include "../includes/includes.hpp"

std::vector<std::string> getSNandPI(const std::string& scriptPath, const std::string& reqURL) {
    std::vector<std::string> result;
    std::string scriptBaseName;
	size_t lastSlash = scriptPath.rfind('/');
	if (lastSlash != std::string::npos)
		scriptBaseName = scriptPath.substr(lastSlash + 1);
	else 
		scriptBaseName = scriptPath;
	std::string pathInfo;
	std::string scriptName;
	size_t scriptPos = reqURL.find(scriptBaseName);
	if (scriptPos != std::string::npos) {
		scriptPos += scriptPath.length();
		scriptName = reqURL.substr(0, scriptPos);

		size_t queryPos = reqURL.find('?', scriptPos);
		if (queryPos != std::string::npos)
			pathInfo = reqURL.substr(scriptPos, queryPos - scriptPos);
		else 
			pathInfo = reqURL.substr(scriptPos);
	}
    result.push_back(scriptName);
    result.push_back(pathInfo);
    return (result);
}