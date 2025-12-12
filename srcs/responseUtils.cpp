#include "../includes/includes.hpp"

void    Response::_initStatusMessages() {
    _statusMessages[200] = "OK";
    _statusMessages[201] = "Created";
    _statusMessages[204] = "No Content";
    _statusMessages[400] = "Bad Request";
    _statusMessages[403] = "Forbidden";
    _statusMessages[404] = "Not Found";
    _statusMessages[405] = "Method Not Allowed";
    _statusMessages[409] = "Conflict";
    _statusMessages[413] = "Content Too Large";
    _statusMessages[414] = "URI Too Large";
    _statusMessages[431] = "Request Header Fields Too Large";
    _statusMessages[500] = "Internal Server Error";
    _statusMessages[501] = "Not Implemented";
    _statusMessages[502] = "Bad Gateway";
    _statusMessages[503] = "Service Unavailable";
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

void    Response::_buildResponse(int statusCode, Request& req, bool upload, const std::string& MIMEType) {
    this->_statusCode = statusCode;
    this->_httpVersion = req.getVersion();

    if (upload)
        this->_content = "<html><body><h1>201Created</h1></body></html>";
    this->_contentSize = this->_content.size();

    _setHeaders(MIMEType);
}

void Response::buildErrorResponse(int code, Request& req, Server& server) {
    this->_statusCode = code;
    this->_httpVersion = req.getVersion();
    
    // Check for a custom error page in the server config.
    std::map<int, std::string>::const_iterator it = server.errorPages.find(code);
    if (it != server.errorPages.end()) { // CHECK FOR ERROR PAGES 
        std::string absolutePath = server.mainRoot + "/" + it->second; // CHECK THE WORKING OF THIS
        if (readFile(it->second, this->_content)) {
            this->_contentSize = this->_content.size();
			_setHeaders("text/html");
            return;
        }
    }
    
    // If no custom page, create a default one.
    std::ostringstream oss;
    std::string statusMessage = "Unknown status";
    std::map<int, std::string>::const_iterator it2 = _statusMessages.find(this->_statusCode);
    if (it2 != _statusMessages.end())
    statusMessage = it2->second;

    oss << "<!DOCTYPE html>\n"
        << "<html lang=\"en\">\n"
        << "<head>\n"
        << "    <meta charset=\"UTF-8\">\n"
        << "    <title>" << this->_statusCode << " " << statusMessage << "</title>\n"
        << "    <style>\n"
        << "        * { margin: 0; padding: 0; box-sizing: border-box; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; }\n"
        << "        body { background: linear-gradient(135deg, #f5f7fa, #c3cfe2); color: #333; display: flex; justify-content: center; align-items: center; height: 100vh; text-align: center; }\n"
        << "        .container { background: rgba(255, 255, 255, 0.9); padding: 50px 80px; border-radius: 15px; box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1); max-width: 600px; }\n"
        << "        h1 { font-size: 3rem; margin-bottom: 20px; color: #d9534f; }\n"
        << "        p { font-size: 1.2rem; color: #555; margin-bottom: 30px; }\n"
        << "        .buttons { display: flex; justify-content: center; gap: 20px; }\n"
        << "        .btn { padding: 15px 30px; font-size: 1.2rem; color: #fff; background-color: #007bff; border: none; border-radius: 5px; text-decoration: none; transition: background-color 0.3s ease; }\n"
        << "        .btn:hover { background-color: #0056b3; }\n"
        << "    </style>\n"
        << "</head>\n"
        << "<body>\n"
        << "    <div class=\"container\">\n"
        << "        <h1>" << this->_statusCode << " - " << statusMessage << "</h1>\n"
        << "        <p>Oops! Something went wrong. The page you were looking for could not be found or an error occurred.</p>\n"
        << "        <div class=\"buttons\">\n"
        << "            <a href=\"/\" class=\"btn\">Go Home</a>\n"
        << "        </div>\n"
        << "    </div>\n"
        << "</body>\n"
        << "</html>";

    this->_content = oss.str();
    this->_contentSize = this->_content.size();
    _setHeaders("text/html");
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
    if (location.empty()) {
        DEBUG_LOG(RED << "Error: " << RESET << "Empty location string in _findRoute");
        return (NULL);
    }
    
    DEBUG_LOG("Trying to find route for location: " << location);
   
    Route*  bestMatch = NULL;
    size_t  longestMatch = 0;

    for (size_t i = 0; i < server.routes.size(); ++i) {
        const std::string& routeLocation = server.routes[i].location;
        if (location.rfind(routeLocation, 0) == 0) {
            if (routeLocation.length() > longestMatch) {
                longestMatch = routeLocation.length();
                bestMatch = &server.routes[i];
            }
        }
    }
    DEBUG_LOG("Best match location = " << bestMatch->location);
    return (bestMatch);
}

void    Response::_constructRelativePath(Request &req, Route* route) {
    std::string path;
    std::string content = req.getContent();
    std::string location = route->location;
    DEBUG_LOG("BEFORE---------" << std::endl << "Content = " << content << std::endl << "Route location = " << route->location);
    
    std::string relativePath = content;
    if (content.rfind(location, 0) == 0) // if content starts with location
        relativePath = content.substr(location.length());

    path = route->root;

    if (!path.empty() && path[path.size() - 1] == '/')
        path.erase(path.size() - 1);

    if (!relativePath.empty() && relativePath[0] != '/') // add a slah if it doesnt start with one
        path += '/';

    path += relativePath;

    DEBUG_LOG(YELLOW << "constructed path = " << path << RESET);

    route->path = path;
}

std::string Response::_generateAutoIndex(const std::string& path, const std::string& reqURL) const {
    //DEBUG_LOG(YELLOW << "AUTOINDEX" << RESET);

    std::ostringstream oss;
    oss << "<!DOCTYPE html>\n";
    oss << "<html lang=\"en\">\n";
    oss << "<head>\n";
    oss << "    <meta charset=\"UTF-8\">\n";
    oss << "    <title>Index of " << reqURL << "</title>\n";
    oss << "    <style>\n";
    oss << "        * {\n";
    oss << "            margin: 0;\n";
    oss << "            padding: 0;\n";
    oss << "            box-sizing: border-box;\n";
    oss << "            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;\n";
    oss << "        }\n";
    oss << "        body {\n";
    oss << "            background: linear-gradient(135deg, #f5f7fa, #c3cfe2);\n";
    oss << "            color: #333;\n";
    oss << "            padding: 40px 20px;\n";
    oss << "        }\n";
    oss << "        .container {\n";
    oss << "            background: rgba(255, 255, 255, 0.9);\n";
    oss << "            padding: 40px;\n";
    oss << "            border-radius: 15px;\n";
    oss << "            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1);\n";
    oss << "            max-width: 800px;\n";
    oss << "            margin: 0 auto;\n";
    oss << "        }\n";
    oss << "        h1 {\n";
    oss << "            text-align: center;\n";
    oss << "            font-size: 2rem;\n";
    oss << "            margin-bottom: 30px;\n";
    oss << "            color: #333;\n";
    oss << "            border-bottom: 1px solid #ddd;\n";
    oss << "            padding-bottom: 20px;\n";
    oss << "        }\n";
    oss << "        ul { list-style-type: none; }\n";
    oss << "        li { padding: 10px 0; border-bottom: 1px solid #eee; }\n";
    oss << "        li:last-child { border-bottom: none; }\n";
    oss << "        a {\n";
    oss << "            text-decoration: none;\n";
    oss << "            color: #0056b3; /* Using a standard blue for links in a list is often more readable */\n";
    oss << "            font-size: 1.1rem;\n";
    oss << "            transition: color 0.2s ease;\n";
    oss << "        }\n";
    oss << "        a:hover { color: #ff4c4c; } /* Use your brand's red color for the hover effect */\n";
    oss << "    </style>\n";
    oss << "</head>\n";
    oss << "<body>\n";
    oss << "    <div class=\"container\">\n";
    oss << "        <h1>Index of " << reqURL << "</h1>\n";
    oss << "        <ul>\n";

    DIR* dir = opendir(path.c_str());
    if (!dir) { return ""; }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name[0] == '.') 
            continue;

        std::string href = reqURL;
        if (!href.empty() && href[href.length() - 1] != '/') {
            href += '/';
        }
        if (name == "..") {
            size_t pos = reqURL.rfind('/', reqURL.length() - 2);
            if (pos != std::string::npos) {
                href = reqURL.substr(0, pos + 1);
            }
        } else {
            href += name;
        }

        std::string fullPath = path + "/" + name;
        struct stat path_stat;
        stat(fullPath.c_str(), &path_stat);
        std::string displayName = name;
        if (S_ISDIR(path_stat.st_mode)) {
            displayName += "/";
        }

        oss << "            <li><a href=\"" << href << "\">" << displayName << "</a></li>\n";
    }
    closedir(dir);

    oss << "        </ul>\n";
    oss << "    </div>\n";
    oss << "</body>\n";
    oss << "</html>\n";
    return oss.str();
}

std::string Response::_generateUploadJSON(const std::string& path) const {
    std::ostringstream oss;
    oss << "[";

    DIR* dir = opendir(path.c_str());
    if (dir) {
        struct dirent* entry;
        bool first = true;
        while ((entry = readdir(dir)) != NULL) {
            std::string name = entry->d_name;
            if (name != "." && name != ".." && name[0] != '.') {
                if (!first)
                    oss << ",";
                oss << "\"" << name << "\"";
                first = false;
            }
        }
        closedir(dir);
    }

    oss << "]";
    return (oss.str());
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
