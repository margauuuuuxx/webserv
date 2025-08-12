#include "../includes/includes.hpp"

/*
	This class is responsible for building the appropriate HTPP response for a given request.
*/

Response::Response() {}

Response::~Response() {}

static void	Response::buildErrorResponse(int code, Request& req, Server& server)
{
	this->_statusCode = code;
	this->_httpVersion = req.getVersion();

	if (server.errorPages(code)) { // CHECK THIS LINE NOT SURE HOW IT WORKS 
		std::string errorPagePath = server.errorPage[code]; // CHECK HOW THE PATH WORKS (I.E. ROOTING PATH)
		std::ifstream file(errorPagePath.c_str());
		if (file.is_open())
		{
			std::string line;
			while (std::getline(file, line)) {
				this->_content += line; // QUID CONTENT SIZE ??
			}
			file.close();
		}
	}

	if (this->_content.empty())
	{
		std::string errorMessage = "An error occured ...";
		if (this->_statusCode == 405)
			errorMessage = "Method not allowed ...";
	
	}
}

static void	Response::handleGET(Request& req, Server& server) {
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
		this->_statusCode = 404;
		this->_httpVersion = req.getVersion();
		this->_contentSize = 0;
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
		this->_statusCode = 404;
		this->_httpVersion = req.getVersion();
		this->_contentSize = 0;
		return ;
	}
	std::string line;
	while (std::getline(file, line)){
		this->_content += line;
	}
	file.close();
	std::cout << "content: " << std::endl;
	std::cout << this->_content << std::endl;
	this->_contentSize = this->_content.size();
	this->_statusCode = 200;
	this->_httpVersion = req.getVersion();
	
}

/*
	POST:
		1. reads the date from the request body
		2. identify what to do with it 
		3. send back a response with status code 
*/
static void	Response::handlePOST(Request& req, Server& server)
{

}

/*
		DELETE:
			1. identify the resource to be deleted
			2. attempts to delete it
			2. sends back a response with status code
*/
static void	Response::handleDELETE(Request& req, Server& server)
{

}

typedef void	(*HandlerFct)(const Request&, Server&);
void	Response::handleRequest(Request& req, Server& server)
{
	std::string method = req.getMethod();	
	this->_httpVersion = req.getVersion();

	std::cout << "===RESPONSE SETUP===" << std::endl;

	std::map<std::string, HandlerFct> handlers;
	handlers["GET"] = &handleGET;
	handlers["POST"] = &handlePOST;
	handlers["DELETE"] = &handleDELETE;

	std::map<std::string, HandlerFct>::iterator it = handlers.find(method);
	if (it != handlers.end())
		it->second(req, server);
	else
		this->_statusCode = 405;
}

std::string	Response::getResponse() {
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
    res << this->_httpVersion << " ";
    num << this->_statusCode;
    res << num.str() << " ";
    if (statusMessages.find(this->_statusCode) != statusMessages.end())
        res << statusMessages[this->_statusCode];
    else
        res << "Unknown";
    res << "\r\n";

    // En-têtes
    std::ostringstream len;
    len << this->_contentSize;
    res << "Content-Length: " << len.str() << "\r\n";
    res << "Content-Type: text/html\r\n";
    res << "Connection: close\r\n";
    res << "\r\n"; // ligne vide

    // Corps
    res << this->_content;

    return res.str();
}
