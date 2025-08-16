#include "../includes/Response.hpp"
#include <map>
Response::Response(){}
Response::~Response(){}

struct location Response::getLocationAndFilename(Request& req, Server& server){
	struct location data;
	std::string location = req.getContent();
	Route* route = NULL;
	std::cout << "location: " << location << std::endl;
	for(size_t i = 0; i < server.routes.size(); i++){
		if (server.routes[i].location == location){
			data.route = &server.routes[i];
			break;
		}
	}
	if (!route){
		size_t len = 0;
		for(size_t i = 0; i < server.routes.size(); i++){
			if (startsWith(location, server.routes[i].location)){
				if (server.routes[i].location.size() > len){
					len = server.routes[i].location.size();
					data.route = &server.routes[i];
					data.filename = location.substr(server.routes[i].location.size());
				}
			}
		}
	}
	if (!data.route) {
		std::cout << "bad location" << std::endl;
		this->_status_code = 404;
		this->_http_version = req.getVersion();
		this->_content_size = 0;
		return data;	
	}
	std::cout << "route: " << data.route->location << std::endl;
	std::cout << "filename: " << data.filename << std::endl;
	return data;
}
bool Response::startsWith(const std::string &str, const std::string &prefix) {
    if (prefix.size() > str.size())
        return false; // un préfixe plus long que la chaîne ne peut pas matcher
    return str.compare(0, prefix.size(), prefix) == 0;
}
void Response::handleGET(Request& req, Server& server, struct location& loc){
	// std::string location = req.getContent();
	// std::string filename;
	// Route* route = NULL;
	// std::cout << "location: " << location << std::endl;
	// for(size_t i = 0; i < server.routes.size(); i++){
	// 	if (server.routes[i].location == location){
	// 		route = &server.routes[i];
	// 		break;
	// 	}
	// }
	// if (!route){
	// 	size_t len = 0;
	// 	for(size_t i = 0; i < server.routes.size(); i++){
	// 		if (startsWith(location, server.routes[i].location)){
	// 			if (server.routes[i].location.size() > len){
	// 				len = server.routes[i].location.size();
	// 				route = &server.routes[i];
	// 				filename = location.substr(server.routes[i].location.size());
	// 			}
	// 		}
	// 	}
	// }
	// if (!route) {
	// 	std::cout << "bad location" << std::endl;
	// 	this->_status_code = 404;
	// 	this->_http_version = req.getVersion();
	// 	this->_content_size = 0;
	// 	return ;	
	// }
	// std::cout << "route: " << route->location << std::endl;
	// std::cout << "filename: " << filename << std::endl;
	std::string filename = loc.filename;
	Route *route = loc.route;

	std::ifstream file;
	bool found = 0;
	if (!filename.empty()) {
		filename = route->root + "/" + filename; 
		file.open(filename.c_str());
		if (file.is_open()){
			std::cout << "good filename: " << filename << std::endl;
			found = 1;
		}
	}
	else{
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
	struct location loc = getLocationAndFilename(req, server);
	std::cout << "===RESPONSE SETUP===" << std::endl;
	if (methode == "GET") {
		std::cout << "GET request" << std::endl;
		handleGET(req, server, loc);	
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
