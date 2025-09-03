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


std::string Response::getResponse() {
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

// ENTRY POINT INTO THE FILE
typedef void (Response::*HandlerFct)(Request&, Server&);
void Response::handleRequest(Request& req, Server& server) {
    this->_httpVersion = req.getVersion();

    // checking the route 
    Route* route = findRoute(req, server);
    if (!route)
    {
        (buildErrorResponse(404, req, server));
        return;
    }

    std::cout << "===RESPONSE SETUP===" << std::endl;

    // checking if the method is allowed
    static std::map<std::string, HandlerFct> handlers;
    if (handlers.empty()) {
        handlers["GET"] = &Response::handleGET;
        handlers["POST"] = &Response::handlePOST;
        handlers["DELETE"] = &Response::handleDELETE;
    }

    std::map<std::string, HandlerFct>::const_iterator it = handlers.find(req.getMethod());
    if (it != handlers.end())
        (this->*(it->second))(req, server);
    else
    {
        buildErrorResponse(405, req, server);
        return;
    }
}