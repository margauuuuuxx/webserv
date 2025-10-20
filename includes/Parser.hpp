#pragma once

#include <vector>
#include <iostream>
#include <exception>
#include <fstream>
#include <sstream>
#include "Server.hpp"

class Parser{
public:
	Parser();
	~Parser();

	int 	parsefile(const std::string& filename);
	void	setAbsoluteRootPath(const std::string& path);
	
	std::vector<Server> getServer() const;

private:
	std::vector<std::string>	_tokens;
	size_t						_i;
	std::vector<Server> 		_servers;
	std::string					_absoluteRootPath;

	void tokenizer(std::ifstream& file);
	std::ifstream openfile(const std::string& filename);
	void cleanComments(std::string& line) const;
	void printTokens() const;

	void expected(const std::string& token);
	void parser();

	void parseServer();
	void parseServerElements(Server& server);

	void parseListen(Server& server);
	void parseHost(Server& server);
	void parseServerName(Server& server);
	void parseErrorPage(Server& server);
	void parseClientMaxBodySize(Server& server);
	bool isValidIPv4(const std::string& ip);

	void parseRoutes(Server& server);
	void parseRouteElements(Route& route);
	void parseRoot(Route& route);
	void parseIndex(Route& route);
	void parseAllowMethods(Route& route);
	void parseAutoIndex(Route& route);
	void parseUploadStore(Route& route);
	void parseUploadEnable(Route& route);
	void parseCgiExtention(Route& route);
	void parseCgiPath(Route& route);

	void printServer() const;
};

