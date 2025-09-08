#include "../includes/includes.hpp"

static std::map<int, std::string> initStatusMessages() {
    std::map<int, std::string> m;
    m[200] = "OK";
    m[201] = "Created";
    m[204] = "No Content";
    m[404] = "Not Found";
    m[405] = "Method Not Allowed";
    m[501] = "Not Implemented";
    return m;
}
const std::map<int, std::string> statusMessages = initStatusMessages();

Response::Response() : _contentSize(0), _statusCode(0) {}

Response::~Response() {}

void    Response::buildResponse(Request& req, Route* route, bool isAutoIndex) {
    this->_statusCode = 200;
    this->_httpVersion = req.getVersion();
    if (isAutoIndex) {
        std::string resourcePath = route->root + req.getContent();
        this->_content = generateAutoIndex(resourcePath, req.getContent()); // END IMPLEMENTATION
    }
    this->_contentSize = this->_content.size();
}


void Response::handleGET(Request& req, Server& server, Route* route) {
    std::string resourcePath = route->root + req.getContent();

    struct stat path_stat;
    if (stat(resourcePath.c_str(), &path_stat) != 0) {
        buildErrorResponse(404, req, server);
        return;
    }

    if (S_ISDIR(path_stat.st_mode))
    {
        bool indexFound = 0;
        for (size_t i = 0; i < route->index.size(); i++) {
            std::string indexPath = resourcePath + "/" + route->index[i];
            std::cout << "indexPath: " << indexPath << std::endl;
            if (access(indexPath.c_str(), F_OK) == 0)
                if (readFile(indexPath, this->_content)) {
                    std::cout << "good indexPath: " << indexPath << std::endl;
                    buildResponse(req, route, 0);
                    indexFound = 1;
                    break;
                }
        }
        if (!indexFound)
            if (route->autoindex)
                buildResponse(req, route, 1);
            else {
                std::cout << "NOT found and no autoindex" << std::endl; // I WANT OUTPUT IN THE TERMINAL
                buildErrorResponse(403, req, server);
            }
    }
    else if (S_ISREG(path_stat.st_mode)) {
        if (isCGIReq(req.getContent(), route)) {
            handleCGI(resourcePath, req, server, route); // IMPLEMENT
            return;
        }
        else {
            if (readFile(resourcePath, this->_content))
                buildResponse(req, route, 0);
            else 
                buildErrorResponse(403, req, server);
        }
    }
    else
        buildErrorResponse(404, req, server);
}

void Response::handlePOST(Request& req, Server& server, Route* route) {

    if (req.getBody().size() > server.clientMaxBodySize)
    {
        buildErrorResponse(413, req, server); // 413 Playload Too Large
        return;
    }

}

void Response::handleDELETE(Request& req, Server& server, Route* route) {
    // Not implemented yet. Responding with an error.
    (void)req; // Suppress unused parameter warning
    (void)server; // Suppress unused parameter warning
    buildErrorResponse(501, req, server); // 501 Not Implemented
}


std::string Response::getResponse() {
    std::ostringstream res;
    std::string statusMessage = "Unknown status";

    std::map<int, std::string>::const_iterator it = statusMessages.find(this->_statusCode);
    if (it != statusMessages.end())
        statusMessage = it->second;

    res << this->_httpVersion << " " << this->_statusCode << " " << statusMessage << "\r\n";
    res << "Content-Length: " << this->_contentSize << "\r\n";
    res << "Content-Type: text/html\r\n";
    res << "Connection: close\r\n";
    res << "\r\n";

    res << this->_content;
    
    return res.str();
}

// ENTRY POINT INTO THE FILE
typedef void (Response::*HandlerFct)(Request&, Server&, Route* route);
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

    static std::map<std::string, HandlerFct> handlers;
    if (handlers.empty()) {
        handlers["GET"] = &Response::handleGET;
        handlers["POST"] = &Response::handlePOST;
        handlers["DELETE"] = &Response::handleDELETE;
    }

    std::string method = req.getMethod();
    std::map<std::string, HandlerFct>::const_iterator it = handlers.find(method);
    if (it != handlers.end()
        && std::find(route->allowedMethods.begin(), route->allowedMethods.end(), method) != route->allowedMethods.end())
        (this->*(it->second))(req, server, route);
    else
    {
        buildErrorResponse(405, req, server);
        return;
    }
}
