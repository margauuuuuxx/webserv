#include "../includes/includes.hpp"

Response::Response() : _contentSize(0), _statusCode(0) { 
    _initMIMETypes(); 
    _initStatusMessages();
}

Response::~Response() {}

void Response::_handleGET(Request& req, Server& server, Route* route) {
    std::string resourcePath = route->path; // PARSER 
    // HAVING THE RIGHT PATH FOR /hello/index.html --> www/index.html

    if (!resourcePath.empty())
        DEBUG_LOG(YELLOW << "GET" << std::endl << RESET << "Route path = " << route->path << std::endl << "Content = " << req.getContent() << std::endl << "GET resource path: " << resourcePath << std::endl << YELLOW << "-----------" << RESET);
    else 
        DEBUG_LOG(RED << "GET resource path empty" << RESET);

    struct stat path_stat;
    if (stat(resourcePath.c_str(), &path_stat) != 0) {
        buildErrorResponse(404, req, server);
        return;
    }

    if (S_ISDIR(path_stat.st_mode)) // dir
    {
        bool indexFound = 0;
        for (size_t i = 0; i < route->index.size(); i++) {
            std::string indexPath = resourcePath + "/" + route->index[i];
            DEBUG_LOG("indexPath: " << indexPath);
            if (access(indexPath.c_str(), F_OK) == 0)
                if (readFile(indexPath, this->_content)) {
                    DEBUG_LOG(RED << "HERE" << RESET);
                    DEBUG_LOG("good indexPath: " << indexPath);
                    _buildResponse(200, req, 0, _getMIMEType(indexPath));
                    indexFound = 1;
                    break;
                }
        }
        if (!indexFound) {
            if (route->autoindex) {
                DEBUG_LOG("AUTOINDEX BLOCK");
                this->_content = _generateAutoIndex(resourcePath, req.getContent());
                if (this->_content.empty()) {
                    DEBUG_LOG(RED << "Error: " << RESET << "Autoindex string empty");
                    buildErrorResponse(500, req, server);
                }
                else 
                    _buildResponse(200, req, 0, "text/html");
            }
            else {
                DEBUG_LOG("NOT found and no autoindex");
                buildErrorResponse(403, req, server);
            }
        }
    }
    else if (S_ISREG(path_stat.st_mode)) { // file 
        if (isCGIReq(req.getContent(), route)) {
            handleCGI(*this, resourcePath, req, server, route);
            return;
        }
        else {
            DEBUG_LOG(RED << "HERE 22" << RESET);
            std::string MIMEType = _getMIMEType(resourcePath);
            if (readFile(resourcePath, this->_content))
                _buildResponse(200, req, 0, MIMEType);
            else 
                buildErrorResponse(403, req, server);
        }
    }
    else
        buildErrorResponse(404, req, server);
}

void Response::_handlePOST(Request& req, Server& server, Route* route) {

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
            _buildResponse(201, req, 1, "text/html");
        }
        else 
            buildErrorResponse(500, req, server);
        return;
    }
    buildErrorResponse(403, req, server);
}

void Response::_handleDELETE(Request& req, Server& server, Route* route) {
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
        _buildResponse(204, req, 0, "text/html");
        DEBUG_LOG("File " << filePath << " deleted successfully");
    } else {
        if (errno == EBUSY) // file is in use/locked
            buildErrorResponse(409, req, server);
        else 
            buildErrorResponse(500, req, server);
    }
}

std::vector<char> Response::getResponse() const {
    std::ostringstream res;
    std::string statusMessage = "Unknown status";

    std::map<int, std::string>::const_iterator it = _statusMessages.find(this->_statusCode);
    if (it != _statusMessages.end())
        statusMessage = it->second;

    res << this->_httpVersion << " " << this->_statusCode << " " << statusMessage << "\r\n";
    std::map<std::string, std::string>::const_iterator it2;
    for (it2 = _headersMap.begin(); it2 != _headersMap.end(); ++it2)
        res << it2->first << ":" << it2->second << "\r\n";
    res << "\r\n";
    res << this->_content;
    
    std::string resStr = res.str();
    std::vector<char> resVector(resStr.begin(), resStr.end());
    // CONVERTIR EN OCTETS 

    return (resVector);
}

// ENTRY POINT INTO THE FILE
typedef void (Response::*HandlerFct)(Request&, Server&, Route* route);
void Response::handleRequest(Request& req, Server& server) {
    this->_httpVersion = req.getVersion();

    DEBUG_LOG(YELLOW << "Location before _findRoute = " << req.getContent() << RESET);

    Route* route = _findRoute(req, server);
    if (!route)
    {
        (buildErrorResponse(404, req, server));
        req.reset();
        DEBUG_LOG(RED << "Error: " << RESET << "No route has been found for " << req.getContent());
        return;
    }
    DEBUG_LOG(YELLOW << "Found route = " << route->location << RESET);

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

    DEBUG_LOG("===RESPONSE SETUP===");

    static std::map<std::string, HandlerFct> handlers;
    if (handlers.empty()) {
        handlers["GET"] = &Response::_handleGET;
        handlers["POST"] = &Response::_handlePOST;
        handlers["DELETE"] = &Response::_handleDELETE;
    }

    std::string method = req.getMethod();
    std::map<std::string, HandlerFct>::const_iterator it = handlers.find(method);
    if (it != handlers.end()
        && std::find(route->allowedMethods.begin(), route->allowedMethods.end(), method) != route->allowedMethods.end())
        (this->*(it->second))(req, server, route);
    else
    {
        buildErrorResponse(405, req, server); // WHAT TO PUT HERE AS MIMETYPE
        req.reset();
        return;
    }
}
