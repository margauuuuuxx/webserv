#pragma once

#include "includes.hpp"

class Request;
class Server;
struct Route;
class CGI;

extern const std::map<int, std::string> statusMessages;

class Response {
public:
    Response();
    ~Response();

    void        handleRequest(Request& req, Server& server);
    std::vector<char> getResponse() const;
    void        buildErrorResponse(int code, Request& req, Server& server);
    void        buildCGIResponse(CGI &CGIobj);

private:
    void                _handleGET(Request& req, Server& server, Route* route);
    void                _handlePOST(Request& req, Server& server, Route* route);
    void                _handleDELETE(Request& req, Server& server, Route* route);
    void                _buildResponse(int statusCode, Request& req, bool upload, const std::string& MIMEType);
    Route*              _findRoute(Request& req, Server &server) const;
    std::string         _generateAutoIndex(const std::string& path, const std::string& reqURL) const;
    std::string         _generateUploadJSON(const std::string& path) const;
    void                _setHeaders(const std::string& MIMEType);
    void                _initMIMETypes();
    void                _initStatusMessages();
    std::string         _getMIMEType(const std::string& path) const;
    void                _constructRelativePath(Request& req, Route* route);

    std::string                         _content;
    std::string                         _httpVersion;
    size_t                              _contentSize;
    size_t                              _statusCode;
    std::map<std::string, std::string>  _headersMap;
    std::map<std::string, std::string>  _MIMETypes;
    std::map<int, std::string>          _statusMessages;
};
