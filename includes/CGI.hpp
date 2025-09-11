#pragma once

#include <string>

class Request;
class Server;
struct Route;
class Response;

class CGI {
    private:
        Request&            _req;
        Server              _server;
        const std::string&  _scriptPath; 
        std::string         _CGIPath;
        std::string         _pathInfo;
        std::string         _scriptName;
        std::string         _queryString;
        const std::string   &_reqURL;
        char**              _envv;
        //std::map<std::string, std::string>  _envvMap;

        CGI();
        void    _parse();
        void    _setEnvv();
        void	_setQueryString();
        void    _setSNandPI();
        void    _setHeaders(std::vector<std::string>);
        void    _vectToArray(std::vector<std::string>);
        
        public:
        CGI(Request &req, Server& server, const std::string& scriptPath);
        ~CGI();
        
        void                freeEnvv();
        void	            handleCGI(Response &res, const std::string& filename, Request& req, Server& server, Route* route);
        void                execute();
        char**              getEnvv() const;
        const std::string&  getCGIPath() const;
        const std::string&  getPathInfo() const;
        const std::string&  getQueryString() const;
};

//std::string CGI(const std::string& filePath,const std::string& body );