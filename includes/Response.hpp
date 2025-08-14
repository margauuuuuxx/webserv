#pragma once

#include "includes.hpp"

// Forward-declare classes to reduce header dependencies
class Request;
class Server;

class Response {
public:
    Response();
    ~Response();

    void        handleRequest(Request& req, Server& server);
    std::string getResponse();

private:
    void handleGET(Request& req, Server& server);
    void handlePOST(Request& req, Server& server);
    void handleDELETE(Request& req, Server& server);
    void buildErrorResponse(int code, Request& req, Server& server);
    Route*  findRoute(const std::string& location, Server &server);

    std::string _content;
    std::string _httpVersion;
    size_t      _contentSize;
    size_t      _statusCode;
};