#include "../includes/includes.hpp"

void    Response::_initStatusMessages() {
    _statusMessages[200] = "OK";
    _statusMessages[201] = "Created";
    _statusMessages[204] = "No Content";
    _statusMessages[404] = "Not Found";
    _statusMessages[405] = "Method Not Allowed";
    _statusMessages[409] = "Conflict";
    _statusMessages[500] = "Internal Server Error";
    _statusMessages[501] = "Not Implemented";
    _statusMessages[504] = "Gateway Timeout";
}

void    Response::_initMIMETypes() {
    _MIMETypes[".html"] = "text/html";
    _MIMETypes[".css"] = "text/css";
    _MIMETypes[".js"] = "application/javascript";
    _MIMETypes[".json"] = "application/json";
    _MIMETypes[".xml"] = "application/xml";
    _MIMETypes[".jpeg"] = "image/jpeg";
    _MIMETypes[".jpg"] = "image/jpeg";
    _MIMETypes[".png"] = "image/png";
    _MIMETypes[".gif"] = "image/gif";
    _MIMETypes[".svg"] = "image/svg+xml";
    _MIMETypes[".ico"] = "image/x-icon";
    _MIMETypes[".pdf"] = "application/pdf";
    _MIMETypes[".txt"] = "text/plain";
}

void    Response::_buildResponse(int statusCode, Request& req, Route* route, bool isAutoIndex, bool upload, const std::string& MIMEType) {
    this->_statusCode = statusCode;
    this->_httpVersion = req.getVersion();
    if (isAutoIndex) {
        std::string resourcePath = route->root + req.getContent();
        this->_content = _generateAutoIndex(resourcePath, req.getContent());
    }
    if (upload)
        this->_content = "<html><body><h1>201Created</h1></body></html>";
    this->_contentSize = this->_content.size();

    _setHeaders(MIMEType);
}

void Response::buildErrorResponse(int code, Request& req, Server& server) {
    this->_statusCode = code;
    this->_httpVersion = req.getVersion();
    _setHeaders("text/html");

    // Check for a custom error page in the server config.
    std::map<int, std::string>::const_iterator it = server.errorPages.find(code);
    if (it != server.errorPages.end()) { // CHECK FOR ERROR PAGES 
        if (readFile(it->second, this->_content)) {
            this->_contentSize = this->_content.size();
            return;
        }
    }

    // If no custom page, create a default one.
    std::ostringstream oss;
    std::string statusMessage = "Unknown status";
    std::map<int, std::string>::const_iterator it2 = _statusMessages.find(this->_statusCode);
    if (it2 != _statusMessages.end())
        statusMessage = it2->second;
    
    oss << "<html>\n<head><title>" << this->_statusCode << " " << statusMessage
        << "</title></head>\n<body>\n<h1>" << this->_statusCode << " " << statusMessage
        << "</h1>\n</body>\n</html>";
    this->_content = oss.str();
    this->_contentSize = this->_content.size();
}

void    Response::buildCGIResponse(CGI& CGIobj) {
    _statusCode = CGIobj.getStatusCode();
    _httpVersion = CGIobj.getHTTPVersion();
    _content = CGIobj.getParsedBody();
    _contentSize = _content.length();
    _headersMap = CGIobj.getHeadersMap();

    std::stringstream ss;
    ss << _contentSize;
    _headersMap["Content-Length"] = ss.str(); // conversion bc HTTP protocol is text-based

    if (_headersMap.find("Content-Type") == _headersMap.end())
        _headersMap["Content-Type"] = "text/html";
}


void    Response::_setHeaders(const std::string& MIMEType) {
    _headersMap.clear();
    std::stringstream ss;
    ss << _contentSize;
    _headersMap["Content-Length"] = ss.str();
    _headersMap["Content-Type"] = MIMEType;
    _headersMap["Connection"] = "close";
}

std::string Response::_getMIMEType(const std::string& path) const {
    size_t  pos = path.rfind('.');
    if (pos != std::string::npos) {
        std::string extension = path.substr(pos);
        std::map<std::string, std::string>::const_iterator it = _MIMETypes.find(extension);
        if (it != _MIMETypes.end())
            return (it->second);
    }
    return ("applications/octet-stream"); // default for unknow file types
}

// Efficiently reads an entire file into a string.
bool readFile(const std::string& path, std::string& content) {
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
        return false;
    }

    file.seekg(0, std::ios::end); // Seek to the end of the file to determine its size
    content.resize(file.tellg());
    file.seekg(0, std::ios::beg); // Seek back to the beginning and read the whole file
    file.read(&content[0], content.size());
    file.close();
    return true;
}


Route*  Response::_findRoute(Request& req, Server &server) const {
    std::string location = req.getContent();
    for (size_t i = 0; i < server.routes.size(); ++i) {
        if (server.routes[i].location == location)
            return &server.routes[i];
    }
    return (NULL);
}

// The goal of this function is to create an HTML page that lists all the files and folders within a given dir, with each entry being a clickable link
std::string Response::_generateAutoIndex(const std::string& path, const std::string& reqURL) const {
    DEBUG_LOG("ReqURL inside generateAutoIndex =    " << reqURL);
    std::ostringstream oss;
    oss << "<html><head><title>Index of " + reqURL + "</title></head><body>";
    oss << "<h1>Index of " << reqURL << "</h1><hr><ul>";

    DEBUG_LOG("Dir path for GenerateAutoIndex =     " << path);
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        DEBUG_LOG(RED << "Error:" << RESET << "GenerateAutoIndex: Cannot open directory: " << path);
        return "";
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;

        std::string fullPath = path;
        if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/')
            fullPath += '/';
        fullPath += name;

        struct stat path_stat;
        if (stat(fullPath.c_str(), &path_stat) != 0) {
            DEBUG_LOG(RED << "Error: " << RESET << "generateAutoIndex: stat()");
            continue;;
        }

        std::string href = reqURL;
        if (!href.empty() && href[href.size() - 1] != '/')
            href += '/';
        href += name;

        std::string displayName = name;
        if (S_ISDIR(path_stat.st_mode))
            displayName += "/";

        oss << "<li><a href=\"" << href << "\">" << displayName << "</a></li>";
    }
        if (closedir(dir) != 0)
            DEBUG_LOG(RED << "Error: " << RESET << "generateAutoIndex: closedir()");

        oss << "</ul><hr></body></html>";
        return oss.str();
}

bool    isCGIReq(const std::string& resource, const Route* route)
{
    if (route->cgiPath.empty())
        return (false);

    size_t dotPos = resource.rfind('.');
    if (dotPos == std::string::npos)
        return (false);

    std::string ext = resource.substr(dotPos);
    for (size_t i = 0; i < route->cgiExtension.size(); ++i) {
        if (ext == route->cgiExtension[i])
            return (true);
    }

    return (false);
}
