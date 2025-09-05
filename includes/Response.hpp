#pragma once

#include "includes.hpp"

// Forward-declare classes to reduce header dependencies
class Request;
class Server;

static const std::map<int, std::string> statusMessages = {
    std::make_pair(200, "OK"),
    std::make_pair(201, "Created"),
    std::make_pair(204, "No content"),
    std::make_pair(404, "Not found"),
    std::make_pair(405, "Method not allowed"),
    std::make_pair(501, "Not implemented")
};

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
    Route*  findRoute(Request& req, Server &server);

    std::string _content;
    std::string _httpVersion;
    size_t      _contentSize;
    size_t      _statusCode;
};