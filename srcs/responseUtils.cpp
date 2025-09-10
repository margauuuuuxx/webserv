#include "../includes/includes.hpp"

// Efficiently reads an entire file into a string.
bool readFile(const std::string& path, std::string& content) {
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
        return false;
    }
    // Seek to the end of the file to determine its size.
    file.seekg(0, std::ios::end);
    content.resize(file.tellg());
    // Seek back to the beginning and read the whole file.
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());
    file.close();
    return true;
}

void Response::buildErrorResponse(int code, Request& req, Server& server) {
    this->_statusCode = code;
    this->_httpVersion = req.getVersion();

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
    std::map<int, std::string>::const_iterator it2 = statusMessages.find(this->_statusCode);
    if (it2 != statusMessages.end())
        statusMessage = it2->second;
    
    oss << "<html>\n<head><title>" << this->_statusCode << " " << statusMessage
        << "</title></head>\n<body>\n<h1>" << this->_statusCode << " " << statusMessage
        << "</h1>\n</body>\n</html>";
    this->_content = oss.str();
    this->_contentSize = this->_content.size();
}

Route*  Response::findRoute(Request& req, Server &server)
{
    std::string location = req.getContent();
    for (size_t i = 0; i < server.routes.size(); ++i) {
        if (server.routes[i].location == location)
            return &server.routes[i];
    }
    return (NULL);
}

// The goal of this function is to create an HTML page that lists all the files and folders within a given dir, with each entry being a clickable link
std::string Response::generateAutoIndex(const std::string& path, const std::string& reqURL) {
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

