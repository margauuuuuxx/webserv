#pragma once

#include "includes.hpp"

struct Route{
	std::string 				location;
	std::vector<std::string>	allowedMethods;
	std::string 				root;
	std::vector<std::string>	index;
	bool 						autoindex;
	std::vector<std::string>	cgiExtension;
	std::string 				cgiPath;
	bool 						uploadEnabled;
	std::string 				uploadStore;
	std::string					path;
	std::vector<bool>			assigned;
	
	Route();
};

class Server{
public:
	Server();
	~Server();

	int 						port;
	std::string 				host;
	std::vector<std::string>	serverNames;
	std::map<int, std::string>	errorPages;  
	int							clientMaxBodySize;
	std::vector<Route>			routes;
	std::map<int, Request>		requests;
	std::string					mainRoot;
	std::vector<bool>			assigned;
	
	void printServerInfos() const;
	
};
