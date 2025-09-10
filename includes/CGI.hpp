#pragma once

#include <string>

class Request;
class Server;
struct Route;
class Response;

void	handleCGI(Response &res, const std::string& filename, Request& req, Server& server, Route* route);
std::string CGI(const std::string& filePath,const std::string& body );