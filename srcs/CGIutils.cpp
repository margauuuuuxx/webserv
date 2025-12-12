#include "../includes/includes.hpp"

void	CGI::_setSNandPI() {
    std::vector<std::string> result;
    std::string scriptBaseName;

	size_t lastSlash = _scriptPath.rfind('/');
	if (lastSlash != std::string::npos)
		scriptBaseName = _scriptPath.substr(lastSlash + 1);
	else 
		scriptBaseName = _scriptPath;
	std::string pathInfo;
	std::string scriptName;
	size_t scriptPos = _reqURL.find(scriptBaseName);
	if (scriptPos != std::string::npos) {
		scriptPos += scriptBaseName.length();
		scriptName = _reqURL.substr(0, scriptPos);

		size_t queryPos = _reqURL.find('?', scriptPos);
		if (queryPos != std::string::npos)
			pathInfo = _reqURL.substr(scriptPos, queryPos - scriptPos);
		else 
			pathInfo = _reqURL.substr(scriptPos);
	}
    _scriptName = scriptName;
    _pathInfo = pathInfo;
}

void	CGI::_setQueryString() {
    size_t pos = _reqURL.find('?');
	std::string substr = "";
	if (pos != std::string::npos)
		substr = _reqURL.substr(pos + 1);
    _queryString = substr;
}

void	CGI::_setHeaders(std::vector<std::string>& v) {
    const std::map<std::string, std::string>& headersMap = _req.getHeaders();
	std::map<std::string, std::string>::const_iterator it;
	for (it = headersMap.begin(); it != headersMap.end(); ++it) {
		std::string key = it->first;
		for (size_t i = 0; i < key.length(); ++i)
			key[i] = std::toupper(key[i]);
		std::replace(key.begin(), key.end(), '-', '_');
		if (key == "CONTENT_TYPE")
			v.push_back(key + "=" + it->second);
		else 
		    v.push_back("HTTP_" + key + "=" + it->second);
	}
}

void	CGI::_vectToArray(const std::vector<std::string>& v) {
	_envv = new char*[v.size() + 1];
	size_t i = 0;
	for (; i < v.size(); ++i)
		_envv[i] = strdup(v[i].c_str());
	_envv[i] = NULL;
}


void	CGI::_readCGI(int fd) {
    char        buffer[4096];
    ssize_t     bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
        _CGIoutput.append(buffer, bytesRead);
    close(fd);
}


int CGI::getStatusCode() { 
    std::map<std::string, std::string>::const_iterator it = _headersMap.find("Status");
    if (it != _headersMap.end())
        return (stringToInt(it->second));
    return (200);
}

std::map<std::string, std::string>  CGI::getHeadersMap() {
    std::map<std::string, std::string>::const_iterator it = _headersMap.find("Status");
    if (it != _headersMap.end())
        _headersMap.erase("Status");
    return (_headersMap);
}

const std::string&  CGI::getParsedBody() { return (this->_parsedBody); }

const std::string&  CGI::getHTTPVersion() { return (_req.getVersion()); }

int  stringToInt(const std::string &s) {
    std::stringstream ss(s);
    int num = 0;
    ss >> num;
    return (num);
}

std::string toString(int nbr) {
    std::ostringstream oss;
    oss << nbr;
    return (oss.str());
}

void    closePipes(int pipe1[2], int pipe2[2]) {
    if (pipe1[0] != -1)
        close(pipe1[0]);
    if (pipe1[1] != -1)
        close(pipe1[1]);
    if (pipe2[0] != -1)
        close(pipe2[0]);
    if (pipe2[1] != -1)
        close(pipe2[1]);
}
