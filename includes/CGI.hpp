#pragma once

#include "includes.hpp"

class Request;
class Server;
struct Route;
class Response;

class CGI {
    private:
        Request&                            _req;
        Server&                             _server;
        const std::string&                  _scriptPath; 
        Response&                           _res;
        std::string                         _CGIPath;
        std::string                         _pathInfo;
        std::string                         _scriptName;
        std::string                         _queryString;
        std::string                         _reqURL;
        char**                              _envv;
        std::string                         _CGIoutput;
        std::string                         _parsedBody;
        std::map<std::string, std::string>  _headersMap;
        pid_t                               _pid;
        int                                 _pipe_out_fd;

        CGI();
        void    _parse();
        void    _setEnvv();
        void	_setQueryString();
        void    _setSNandPI();
        void    _setHeaders(std::vector<std::string>& v);
        void    _vectToArray(const std::vector<std::string>& v);
        void    _readCGI(int fd);
        
        public:
        CGI(Request &req, Server& server, const std::string& scriptPath, Response &res);
        ~CGI();
        
        void	execute(Route* /*route*/);
        int                                 getStatusCode();
        std::map<std::string, std::string>  getHeadersMap();
        const std::string&                  getParsedBody();
        const std::string&                  getHTTPVersion();
};

void	handleCGI(Response& res, const std::string& filename, Request& req, Server& server, Route* route);
