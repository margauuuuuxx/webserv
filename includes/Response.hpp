#pragma once

#include "includes.hpp"

class Request;
class Server;
struct Route;

extern const std::map<int, std::string> statusMessages;

class Response {
public:
    Response();
    ~Response();

    void                    handleRequest(Request& req, Server& server);
    std::vector<char>       getResponse() const;
    void                    buildErrorResponse(int code, Request& req, Server& server);
    void                    initiateFileSend(const std::string& filePath, Request& req, Server& server);
    bool                    isChunkingActive() const;
    void                    prepareNextChunk(size_t maxChunkSize);
    const std::vector<char> &getResponseBuffer() const;
    void                    consumeBufferBytes(size_t bytesSent);
    bool                    isCGI() const;
    int                     getCGIPipeFd() const;
    pid_t                   getCGIPid() const;
    void                    handleCGI();

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
    std::ifstream                       _fileStream;
    bool                                _isChunkingActive;
    std::vector<char>                   _responseBuffer;
    CGI*                                _CGI;
    pid_t                               _CGI_pid;
    int                                 _CGI_pipe_fd;
    bool                                _isCGI;
    void                                _startCGI(const std::string& scriptPath, Request& req, Server& server, Route* route);
};
