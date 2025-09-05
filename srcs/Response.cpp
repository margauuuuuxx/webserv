#include "../includes/Response.hpp"
#include <dirent.h>   // opendir, readdir, closedir
#include <sys/types.h>
#include <string>
#include <sstream>
#include <iostream>
#include <map>
Response::Response(){}
Response::~Response(){}

std::string Response::generateDirectoryListing(const std::string &path, const std::string &uri) {
    DIR *dir;
    struct dirent *entry;
    std::ostringstream html;

    // Début de la page HTML
    html << "<!DOCTYPE html>\n<html>\n<head>\n";
    html << "<meta charset=\"UTF-8\">\n<title>Index of " << uri << "</title>\n";
    html << "</head>\n<body>\n";
    html << "<h1>Index of " << uri << "</h1>\n<ul>\n";

    // Ouvrir le dossier
    dir = opendir(path.c_str());
    if (!dir) {
        return "<html><body><h1>403 Forbidden</h1></body></html>";
    }

    // Lire chaque fichier
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;

        // Ignorer "." et ".."
        if (name == "." || name == "..")
            continue;

        // Ajouter un lien vers le fichier
        html << "<li><a href=\"" << uri;
        if (uri[uri.size() - 1] != '/')
            html << "/";
        html << name << "\">" << name << "</a></li>\n";
    }

    closedir(dir);

    // Fin de la page HTML
    html << "</ul>\n</body>\n</html>\n";

    return html.str();
}
std::string Response::getHTTPDate() {
    // Obtenir l'heure actuelle
    std::time_t now = std::time(NULL);

    // Convertir en structure GMT (UTC)
    std::tm *gmt = std::gmtime(&now);

    // Tampon pour stocker la date formatée
    char buffer[100];

    // Format RFC 1123 pour HTTP
    // Ex: "Tue, 26 Aug 2025 15:30:00 GMT"
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);

    return std::string(buffer);
}

void Response::setContentType(std::string filename){
	this->_content_type = "application/octet-stream";
	if (filename.find(".html") != std::string::npos) 
		_content_type = "text/html";
	else if (filename.find(".png")  != std::string::npos)
		_content_type = "image/png";
	else if (filename.find(".jpg")  != std::string::npos || filename.find(".jpeg") != std::string::npos) 
		_content_type = "image/jpeg";
	else if (filename.find(".gif")  != std::string::npos)
		_content_type = "image/gif";

}

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
	std::string filename = loc.filename;
	Route *route = loc.route;

	std::ifstream file;
	bool found = 0;
	if (!filename.empty()) {
		filename = route->root + "/" + filename; 
		file.open(filename.c_str(), std::ios::in | std::ios::binary);
		if (file.is_open()){
			std::cout << "good filename: " << filename << std::endl;
			setContentType(filename);
			found = 1;
		}
	}
	else{
		for (size_t i = 0; i < route->index.size(); i++) {
			filename = route->root + "/" + route->index[i]; 
			std::cout << "filename: " << filename << std::endl;
			file.open(filename.c_str(), std::ios::in | std::ios::binary);
			if (file.is_open()){
				std::cout << "good filename: " << filename << std::endl;
				setContentType(filename);
				found = 1;
				break;
			}
		}
		if (!found){
			if (route->autoindex){
				std::string html = generateDirectoryListing(route->root, "/");
				this->_content.assign(html.begin(), html.end());
				this->_content_size = this->_content.size();
				this->_status_code = 200;
				this->_http_version = req.getVersion();
				return;
			}
			else{
				this->_status_code = 403;
				this->_http_version = req.getVersion();
				this->_content_size = 0;
				return ;

			}
		}
	}
	//need protection in case of bad filename 
	if (!found){
		this->_status_code = 404;
		this->_http_version = req.getVersion();
		this->_content_size = 0;
		return ;
	}

// Aller à la fin pour avoir la taille
    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);

	this->_content.resize(size);
    file.read(&this->_content[0], size);
    file.close();

	this->_content_size = this->_content.size();
	this->_status_code = 200;
	this->_http_version = req.getVersion();
}

void Response::handleRequest(Request& req, Server& server){
	std::string methode = req.getMethod();	
	struct location loc = getLocationAndFilename(req, server);
	std::cout << "===RESPONSE SETUP===" << std::endl;
	if (methode == "GET") {
		std::cout << "===GET===" << std::endl;
		handleGET(req, server, loc);	
	}
}

#include <map>
#include <sstream>

std::vector<char> Response::getResponse() {
    // Table de correspondance code → message
	static std::map<size_t, std::string> statusMessages;
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

    std::ostringstream header;

    // Ligne de statut
    header << this->_http_version << " " << this->_status_code << " ";
    if (statusMessages.find(this->_status_code) != statusMessages.end())
        header << statusMessages[this->_status_code];
    else
        header << "Unknown";
    header << "\r\n";

    // En-têtes
    header << "Content-Length: " << this->_content_size << "\r\n";
    header << "Content-Type: " << this->_content_type << "\r\n";
	header << "Date: " << getHTTPDate()<< "\r\n";
    header << "Connection: close\r\n";
    header << "\r\n"; // ligne vide

    // Convertir header en std::string
    std::string headerStr = header.str();

    // Construire le vecteur final (header + body)
    std::vector<char> response;
    response.reserve(headerStr.size() + this->_content.size());

    // Copier header
    response.insert(response.end(), headerStr.begin(), headerStr.end());

    // Copier body (binaire)
    response.insert(response.end(), this->_content.begin(), this->_content.end());

    return response;
}
