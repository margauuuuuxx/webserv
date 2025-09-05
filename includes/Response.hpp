#pragma once

#include "includes.hpp"

// Forward-declare classes to reduce header dependencies
class Request;
class Server;

const std::map<int, std::string> statusMessages;
extern const std::map<int, std::string> statusMessages;

class Response {
public:
    Response();
    ~Response();

    void        handleRequest(Request& req, Server& server);
    std::string getResponse();

private:
    void handleGET(Request& req, Server& server, Route* route);
    void handlePOST(Request& req, Server& server, Route* route);
    void handleDELETE(Request& req, Server& server, Route* route);
    void buildErrorResponse(int code, Request& req, Server& server);
    void    buildResponse(Request& req, Route* route, bool isAutoIndex);
    Route*  findRoute(Request& req, Server &server);

    std::string _content;
    std::string _httpVersion;
    size_t      _contentSize;
    size_t      _statusCode;
};