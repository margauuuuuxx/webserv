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
    this->_contentSize = this->_content.size();
    if (isAutoIndex)
        this->_content = generateAutoIndex(route->location); // IMPLEMENT
}


void Response::handleGET(Request& req, Server& server, Route* route) {
    if (isDir(route->location))
    {
        for (size_t i = 0; i < route->index.size(); i++) {
            std::string filename = route->root + "/" + route->index[i];
            std::cout << "filename: " << filename << std::endl;
            if (readFile(filename, this->_content)) {
                std::cout << "good filename: " << filename << std::endl;
                buildResponse(req, route, 0);
                return;
            }
        }
        if (route->autoindex)
        {
            buildResponse(req, route, 1);
            return;
        }
        else {
            std::cout << "NOT found and no autoindex" << std::endl; // I WANT OUTPUT IN THE TERMINAL
            buildErrorResponse(403, req, server);
            return;
        }
    }
    else if (isFile(route->location)) {
        std::string filename = route->root + "/" + route->location;
        if (isCGIReq(filename, route)) {
            handleCGI(filename, req, server, route); // IMPLEMENT
            return;
        }
        else if (readFile(filename, this->_content)) {
            std::cout << "good filename: " << filename << std::endl;
            buildResponse(req, route, 0);
            return;
        }
        else {
            buildErrorResponse(404, req, server);
            return;
        }
    }
    else {
        buildErrorResponse(404, req, server);
        return;
    }
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
