#include "../includes/includes.hpp"

Response::Response() : _contentSize(0), _statusCode(0) {}

Response::~Response() {}

// Efficiently reads an entire file into a string.
static bool readFile(const std::string& path, std::string& content) {
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

Route*  Response::findRoute(const std::string location, Server &server)
{
    for (std::vector<Route>::const_iterator it = server.routes.begin(); it != server.routes.end(); ++it) {
        if (it.location == location) 
            return (route = &(*it));
    }

    return (NULL);    
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

bool    isCGIReq(const std::string& resource, const Route* route)
{
    if (route.cgiPath.empty())
        return (false);

    size_t dotPos = resource.rfind('.');
    if (dotPos == std::string::npos)
        return (false);

    std::string ext = resource.substr(dotPos);
    for (size_t i = 0; i < route.cgiExtension.size(); ++i) {
        if (ext == route.cgiExtension[i])
            return (true);
    }

    return (false);
}

void    handleRoute(Request& req, Server& server, Route* route)
{
    std::string resource = req.getContent();

    if (isCGIReq(resource, route))
        handleCGI(req, server, route);

    else if (route->uploadEnabled)
        handleUpload(req, server, route);
    else
        handleStaticFile(req, server, route);
}

void Response::handleGET(Request& req, Server& server) {
    Route* route = findRoute(req.getContent(), server);
    
    if (!route) {
        std::cout << "bad location" << std::endl;
        return buildErrorResponse(404, req, server);
    }

    std::cout << "route: " << route->location << std::endl;

    for (size_t i = 0; i < route->index.size(); i++) {
        std::string filename = route->root + "/" + route->index[i];
        std::cout << "filename: " << filename << std::endl;
        if (readFile(filename, this->_content)) {
            std::cout << "good filename: " << filename << std::endl;
            this->_statusCode = 200;
            this->_httpVersion = req.getVersion();
            this->_contentSize = this->_content.size();
            return; // Success
        }
    }

    // If no index file was found or readable
    std::cout << "NOT found" << std::endl;
    return buildErrorResponse(404, req, server);
}

void Response::handlePOST(Request& req, Server& server) {
    Route* route = findRoute(req.getContent(), server);

    if (!route)
    {
        buildErrorResponse(404, req, server);
        return;
    }

    if (req.getBody().size() > server.clientMaxBodySize)
    {
        buildErrorResponse(413, req, server); // 413 Playload Too Large
        return;
    }

    

}

void Response::handleDELETE(Request& req, Server& server) {
    // Not implemented yet. Responding with an error.
    (void)req; // Suppress unused parameter warning
    (void)server; // Suppress unused parameter warning
    buildErrorResponse(501, req, server); // 501 Not Implemented
}

typedef void (Response::*HandlerFct)(Request&, Server&);
void Response::handleRequest(Request& req, Server& server) {
    this->_httpVersion = req.getVersion();

    std::cout << "===RESPONSE SETUP===" << std::endl;

    std::map<std::string, HandlerFct> handlers;
    handlers["GET"] = &Response::handleGET;
    handlers["POST"] = &Response::handlePOST;
    handlers["DELETE"] = &Response::handleDELETE;

    std::string method = req.getMethod();
    if (handlers.count(method)) {
        (this->*(handlers[method]))(req, server);

    std::map<std::string, HandlerFct>::const_iterator it = handlers.find(req.getMethod());
    if (it != handlers.end())
        (this->*(it->second))(req, server);
    else
        buildErrorResponse(405, req, server);
}

std::string Response::getResponse() {
    static std::map<size_t, std::string> statusMessages;
    if (statusMessages.empty()) {
        statusMessages[200] = "OK";
        statusMessages[201] = "Created";
        statusMessages[204] = "No Content";
        statusMessages[404] = "Not Found";
        statusMessages[405] = "Method Not Allowed";
        statusMessages[501] = "Not Implemented";
    }

    std::ostringstream res;
    res << this->_httpVersion << " " << this->_statusCode << " ";
    if (statusMessages.count(this->_statusCode)) {
        res << statusMessages[this->_statusCode];
    } else {
        res << "Unknown Status";
    }
    res << "\r\n";

    res << "Content-Length: " << this->_contentSize << "\r\n";
    res << "Content-Type: text/html\r\n";
    res << "Connection: close\r\n";
    res << "\r\n";

    res << this->_content;

    return res.str();
}
