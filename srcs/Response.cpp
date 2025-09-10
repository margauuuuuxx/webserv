#include "../includes/includes.hpp"

static std::map<int, std::string> initStatusMessages() {
    std::map<int, std::string> m;
    m[200] = "OK";
    m[201] = "Created";
    m[204] = "No Content";
    m[404] = "Not Found";
    m[405] = "Method Not Allowed";
    m[409] = "Conflict";
    m[500] = "Internal Server Error";
    m[501] = "Not Implemented";
    return m;
}
const std::map<int, std::string> statusMessages = initStatusMessages();

Response::Response() : _contentSize(0), _statusCode(0) {}

Response::~Response() {}

void    Response::buildResponse(int statusCode, Request& req, Route* route, bool isAutoIndex, bool upload) {
    this->_statusCode = statusCode;
    this->_httpVersion = req.getVersion();
    if (isAutoIndex) {
        std::string resourcePath = route->root + req.getContent();
        this->_content = generateAutoIndex(resourcePath, req.getContent());
    }
    if (upload)
        this->_content = "<html><body><h1>201Created</h1></body></html>";
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
            DEBUG_LOG("indexPath: " << indexPath);
            if (access(indexPath.c_str(), F_OK) == 0)
                if (readFile(indexPath, this->_content)) {
                    DEBUG_LOG("good indexPath: " << indexPath);
                    buildResponse(200, req, route, 0, 0);
                    indexFound = 1;
                    break;
                }
        }
        if (!indexFound) {
            if (route->autoindex) {
                std::string resourcePath = route->root + req.getContent();
                std::string autoIndexContent = generateAutoIndex(resourcePath, req.getContent());
                if (autoIndexContent.empty()) {
                    DEBUG_LOG("Autoindex string empty");
                    buildErrorResponse(500, req, server);
                }
                else 
                    buildResponse(200, req, route, 1, 0);
            }
            else {
                DEBUG_LOG("NOT found and no autoindex");
                buildErrorResponse(403, req, server);
            }
        }
    }
    else if (S_ISREG(path_stat.st_mode)) {
        if (isCGIReq(req.getContent(), route)) {
            handleCGI(*this, resourcePath, req, server, route);
            return;
        }
        else {
            if (readFile(resourcePath, this->_content))
                buildResponse(200, req, route, 0, 0);
            else 
                buildErrorResponse(403, req, server);
        }
    }
    else
        buildErrorResponse(404, req, server);
}

void Response::handlePOST(Request& req, Server& server, Route* route) {

    if (req.getBody().size() > static_cast<size_t>(server.clientMaxBodySize))
    {
        buildErrorResponse(413, req, server);
        return;
    }

    if (isCGIReq(req.getContent(), route)) {
        std::string filename = route->root + req.getContent();
        handleCGI(*this, filename, req, server, route);
        return;
    }

    if (route->uploadEnabled) {
        std::stringstream filename_ss;
        filename_ss << "upload_" << time(NULL);
        std::string filePath = route->uploadStore + "/"  + filename_ss.str();

        std::ofstream newFile(filePath.c_str(), std::ios::binary);
        if (newFile.is_open()) {
            newFile.write(req.getBody().c_str(), req.getBody().length());
            newFile.close();
            buildResponse(201, req, route, 0, 1);
        }
        else 
            buildErrorResponse(500, req, server);
        return;
    }
    buildErrorResponse(403, req, server);
}

void Response::handleDELETE(Request& req, Server& server, Route* route) {
    std::string filePath = route->root + req.getContent();

    struct stat path_stat;
    if (stat(filePath.c_str(), &path_stat) != 0) {
        buildErrorResponse(404, req, server);
        return;
    }

    if (!S_ISREG(path_stat.st_mode) || access(filePath.c_str(), W_OK) != 0) {
        buildErrorResponse(403, req, server);
        return;
    }

    if (remove(filePath.c_str()) == 0) {
        buildResponse(204, req, route, 0, 0);
        DEBUG_LOG("File " << filePath << " deleted successfully");
    } else {
        if (errno == EBUSY) // file is in use/locked
            buildErrorResponse(409, req, server);
        else 
            buildErrorResponse(500, req, server);
    }
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

    Route* route = findRoute(req, server);
    if (!route)
    {
        (buildErrorResponse(404, req, server));
        return;
    }

    DEBUG_LOG("===RESPONSE SETUP===");

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
