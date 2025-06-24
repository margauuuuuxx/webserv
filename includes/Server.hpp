#pragma once

#include <string>
#include <vector>
#include <map>
#include <vector>
#include <iostream>

struct Route{
	std::string location;
	std::vector<std::string> allow_methods;
	std::string root;//directory or file where the requested file should be located
	std::vector<std::string> index;
	bool autoindex;//allow file listing
	std::vector<std::string> cgi_extension;
	std::string cgi_path;
	bool upload_enable;//alow upload files
	std::string upload_store;// where upload files should be saved
};

class Server{
public:
	int port;
	std::string host;
	std::vector<std::string> server_names;
	std::map<int, std::string> error_pages;  
	int client_max_body_size;
	std::vector<Route> routes;
	void printServerInfos() const;
};

