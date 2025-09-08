#pragma once

void	handleCGI(std::string filename, Request& req, Server& server, Route* route);
std::string CGI(const std::string& filePath,const std::string& body );