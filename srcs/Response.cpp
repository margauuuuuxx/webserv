#include "../includes/Response.hpp"
#include <map>
Response::Response(){}
Response::~Response(){}
void Response::handleGET(Request& req, Server& server){
	std::string location = req.getContent();
	Route* route = NULL;
	std::cout << "location: " << location << std::endl;
	for(size_t i = 0; i < server.routes.size(); i++){
		if (server.routes[i].location == location){
			route = &server.routes[i];
			break;
		}
	}
	if (!route) {
		std::cout << "bad location" << std::endl;
		this->_status_code = 404;
		this->_http_version = req.getVersion();
		this->_content_size = 0;
		return ;	
	}
	std::cout << "route: " << route->location << std::endl;

	std::ifstream file;
	std::string filename;
	bool found = 0;
	for (size_t i = 0; i < route->index.size(); i++) {
		 filename = route->root + "/" + route->index[i]; 
		std::cout << "filename: " << filename << std::endl;
		file.open(filename.c_str());
		if (file.is_open()){
			std::cout << "good filename: " << filename << std::endl;
			found = 1;
			break;
		}
	}
	//need protection in case of bad filename 
	if (!found){
		std::cout << "NOT found" << std::endl;
		this->_status_code = 404;
		this->_http_version = req.getVersion();
		this->_content_size = 0;
		return ;
	}
	std::string line;
	while (std::getline(file, line)){
		this->_content += line;
	}
	file.close();
	std::cout << "content: " << std::endl;
	std::cout << this->_content << std::endl;
	this->_content_size = this->_content.size();
	this->_status_code = 200;
	this->_http_version = req.getVersion();
	
}
void Response::handleRequest(Request& req, Server& server){
	std::string methode = req.getMethod();	
	if (methode == "GET") {
		std::cout << "GET request" << std::endl;
		handleGET(req, server);	
	}
}

#include <map>
#include <sstream>

std::string Response::getResponse() {
    // Table de correspondance code → message
    static std::map<size_t, std::string> statusMessages;
    if (statusMessages.empty()) {
        statusMessages[200] = "OK";
        statusMessages[201] = "Created";
        statusMessages[204] = "No Content";
        statusMessages[301] = "Moved Permanently";
        statusMessages[302] = "Found";
        statusMessages[400] = "Bad Request";
        statusMessages[403] = "Forbidden";
        statusMessages[404] = "Not Found";
        statusMessages[405] = "Method Not Allowed";
        statusMessages[500] = "Internal Server Error";
        statusMessages[501] = "Not Implemented";
        statusMessages[502] = "Bad Gateway";
        statusMessages[503] = "Service Unavailable";
    }

    std::ostringstream res;
    std::ostringstream num;

    // Ligne de statut
    res << this->_http_version << " ";
    num << this->_status_code;
    res << num.str() << " ";
    if (statusMessages.find(this->_status_code) != statusMessages.end())
        res << statusMessages[this->_status_code];
    else
        res << "Unknown";
    res << "\r\n";

    // En-têtes
    std::ostringstream len;
    len << this->_content_size;
    res << "Content-Length: " << len.str() << "\r\n";
    res << "Content-Type: text/html\r\n";
    res << "Connection: close\r\n";
    res << "\r\n"; // ligne vide

    // Corps
    res << this->_content;

    return res.str();
}
