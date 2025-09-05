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
    oss << "<html>\n<head><title>" << this->_statusCode << " " << statusMessages[this->_statusCode]
        << "</title></head>\n<body>\n<h1>" << this->_statusCode << " " << statusMessages[this->_statusCode]
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

bool    isFile(const std::string& path)
{
    struct stat info;

    if (stat(path.c_str(), &info) != 0)
    {
        std::cout << "isFile: could not access file"; // CHECK HOW TO THROW ERROR MESSAGES 
        return (0);
    }

    return (info.st_mode & S_IFMT) == S_IFREG;
}

bool    isDir(const std::string& path)
{
    struct stat info;

    if (stat(path.c_str(), &info) != 0)
    {
        std::cout << "isFile: could not access file"; // CHECK HOW TO THROW ERROR MESSAGES 
        return (0);
    }

    return (info.st_mode & S_IFMT) == S_IFDIR;
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

