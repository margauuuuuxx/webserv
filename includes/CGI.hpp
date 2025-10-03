#pragma once

#include "includes.hpp"

class Request;
class Server;
struct Route;

class CGI {
    private:
        Request&                            _req;
        Server&                             _server;
        const std::string&                  _scriptPath; 
        std::string                         _pathInfo;
        std::string                         _scriptName;
        std::string                         _queryString;
        std::string                         _reqURL;
        char**                              _envv;
        pid_t                               _pid;
        int                                 _pipe_out_fd;
        Route*                              _route;

        CGI();
        void    _setEnvv();
        void	_setQueryString();
        void    _setSNandPI();
        void    _setHeaders(std::vector<std::string>& v);
        void    _vectToArray(const std::vector<std::string>& v);
        
        public:
        CGI(Request &req, Server& server, const std::string& scriptPath, Route* route);
        ~CGI();
        
        pid_t   execute();
        pid_t   getPid() const;
        int     getPipeReadFd() const;
};
