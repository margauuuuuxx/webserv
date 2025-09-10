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
		scriptPos += scriptBaseName.length();
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

std::string getQueryString(const std::string& reqURL) {
    size_t pos = reqURL.find('?');
	std::string substr = "";
	if (pos != std::string::npos)
		substr = reqURL.substr(pos + 1);
    return (substr);
}

void    getHeaders(std::vector<std::string>& envVector, Request &req) {
    const std::map<std::string, std::string>& headersMap = req.getHeaders();
	std::map<std::string, std::string>::const_iterator it;
	for (it = headersMap.begin(); it != headersMap.end(); ++it) {
		std::string key = it->first;
		for (size_t i = 0; i < key.length(); ++i)
			key[i] = std::toupper(key[i]);
		std::replace(key.begin(), key.end(), '-', '_');
		if (key == "CONTENT_TYPE")
			envVector.push_back(key + "=" + it->second);
		else 
		    envVector.push_back("HTTP_" + key + "=" + it->second);
	}
}

char**  vectToArray(const std::vector<std::string>& v) {
    char** arr = new char*[v.size() + 1]; // FREEEEEEEE
	size_t i = 0;
	for (; i < v.size(); ++i)
		arr[i] = strdup(v[i].c_str());
	arr[i] = NULL;
    return (arr);
}