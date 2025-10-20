#include "../includes/includes.hpp"

Response::Response() : _contentSize(0), _statusCode(0) { 
    _initMIMETypes(); 
    _initStatusMessages();
}

Response::~Response() {}

void Response::_handleGET(Request& req, Server& server, Route* route) {
    std::string resourcePath = route->path;

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
        if (req.getContent() == "/uploads/") {
            this->_content = _generateUploadJSON(resourcePath);
            _buildResponse(200, req, 0, "application/json");
            return;
        }

        bool indexFound = 0;
        for (size_t i = 0; i < route->index.size(); i++) {
            std::string indexPath = resourcePath + "/" + route->index[i];
            DEBUG_LOG("indexPath: " << indexPath);
            if (access(indexPath.c_str(), F_OK) == 0)
                if (readFile(indexPath, this->_content)) {
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
                buildErrorResponse(404, req, server);
            }
        }
    }
    else if (S_ISREG(path_stat.st_mode)) { // file 
        if (isCGIReq(req.getContent(), route)) {
            handleCGI(*this, resourcePath, req, server, route);
            return;
        }
        else {
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
    DEBUG_LOG(YELLOW << "In the handlePOST fct" << RESET);

    std::string bodySize = intToString(server.clientMaxBodySize);
    DEBUG_LOG("Content len = " << req.getContentLen() << std::endl << "Client max body size = " << bodySize << std::endl);

    if (req.getContentLen() > static_cast<size_t>(server.clientMaxBodySize)) {
        buildErrorResponse(413, req, server);
        return;
    }

    if (isCGIReq(req.getContent(), route)) {
        std::string filename = route->root + req.getContent();
        handleCGI(*this, filename, req, server, route);
        return;
    }

    if (route->uploadEnabled) {
        DEBUG_LOG(YELLOW << "HERE: uploadEnabled" << RESET);

        const std::vector<char>& body_vec = req.getBody();
        std::string body(body_vec.begin(), body_vec.end());

        // Finding the original filename extension
        std::string filename_key = "filename=\"";
        size_t filename_pos = body.find(filename_key);
        if (filename_pos == std::string::npos) {
            DEBUG_LOG(RED << "Error: " << RESET << "Could not find 'filename' in POST body");
            buildErrorResponse(400, req, server); // Bad Request
            return;
        }
        filename_pos += filename_key.length();
        size_t filename_end_pos = body.find("\"", filename_pos);
        if (filename_end_pos == std::string::npos) {
            DEBUG_LOG(RED << "Error: " << RESET << "Could not find closing quote for 'filename'");
            buildErrorResponse(400, req, server); // Bad Request
            return;
        }
        std::string original_filename = body.substr(filename_pos, filename_end_pos - filename_pos);

        std::string extension = "";
        size_t dot_pos = original_filename.rfind(".");
        if (dot_pos != std::string::npos) 
            extension = original_filename.substr(dot_pos);

        // Finding the start of the actual file data
        std::string seperator = "\r\n\r\n";
        size_t data_start_pos = body.find(seperator, filename_end_pos);
        if (data_start_pos == std::string::npos) {
            DEBUG_LOG(RED << "Error: " << RESET << "Could not find data seperator in POST body");
            buildErrorResponse(400, req, server); // Bad Request
            return;
        }
        data_start_pos += seperator.length();

        // Find the end of the file data
        std::string boundary_key = "\r\n--";
        size_t data_end_pos = body.rfind(boundary_key);
        if (data_end_pos == std::string::npos || data_end_pos < data_start_pos) {
            DEBUG_LOG(RED << "Error: " << RESET << "Could not find end boundary in POST body");
            buildErrorResponse(400, req, server); // Bad Request
            return;
        }

        // Creating a new filename and saving the data
        std::stringstream filename_ss;
        filename_ss << "upload_" << time(NULL) << extension;
        std::string filePath = route->uploadStore + "/"  + filename_ss.str();

        std::ofstream newFile(filePath.c_str(), std::ios::binary);
        if (newFile.is_open()) {
            newFile.write(body.c_str() + data_start_pos, data_end_pos - data_start_pos);
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
    std::string filePath = route->path;

    if (filePath.length() >= 5 && filePath.substr(filePath.length() - 5) == ".html") {
        DEBUG_LOG("Attempt to delete an HTML file denied");
        buildErrorResponse(403, req, server); // Forbidden
        return;
    }

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

    return (resVector);
}

// ENTRY POINT INTO THE FILE
typedef void (Response::*HandlerFct)(Request&, Server&, Route* route);
void Response::handleRequest(Request& req, Server& server) {
    this->_httpVersion = req.getVersion();

    Route* route = _findRoute(req, server);
    if (!route)
    {
        (buildErrorResponse(404, req, server));
        req.reset();
        DEBUG_LOG(RED << "Error: " << RESET << "No route has been found for " << req.getContent());
        return;
    }
    DEBUG_LOG(YELLOW << "Found route = " << route->location << RESET);

    _constructRelativePath(req, route);

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
        && std::find(route->allowedMethods.begin(), route->allowedMethods.end(), method) != route->allowedMethods.end()){
        (this->*(it->second))(req, server, route);
	}
    else
    {
        buildErrorResponse(405, req, server);
        req.reset();
        return;
    }
}
