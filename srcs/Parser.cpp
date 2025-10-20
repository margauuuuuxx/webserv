
#include "../includes/Parser.hpp"
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdlib>

Parser::Parser(){}
Parser::~Parser(){}
int Parser::parsefile(const std::string& filename){
	try{
		std::ifstream file(filename.c_str());
		if (!file.is_open()) {
			throw std::runtime_error("Cannot open config file: " + filename);
		}
		tokenizer(file);
		// printTokens();
		// get the absolute path
		char cwd_buffer[1024];
		if (getcwd(cwd_buffer, sizeof(cwd_buffer)) == NULL) {
			DEBUG_LOG(RED << "Error: " << RESET << "main: Could not get cwd");
		}
		//dans serveur il faut absolute rootpath
		std::string	absoluteRootPath = cwd_buffer;
		setAbsoluteRootPath(absoluteRootPath);
		parser();
		printServer();
		return 0;
	}catch(std::exception& e){
		std::cout << "Parsing error: " << e.what() << std::endl;
		return 1;
	}
}

void Parser::tokenizer(std::ifstream& file){
	 std::string line;

	while (std::getline(file, line)){
		cleanComments(line);
		for (size_t i = 0; i < line.size(); i++){
			if (line[i] == '{' || line[i] == '}' || line[i] == ';'){
				line.insert(i, " "); 
				i += 2; 
				line.insert(i, " "); 
				i++;
			}
		}
		std::istringstream iss(line);
		std::string token;
		while (iss >> token) {
			_tokens.push_back(token);
		}
	}
}

void Parser::cleanComments(std::string& line) const {
	size_t pos = line.find("#");
	if (pos != std::string::npos) {
		line = line.substr(0, pos);
	}
}


void Parser::printTokens() const {
    for (size_t i = 0; i < _tokens.size(); i++) {
        std::cout << "[" << i << "] '" << _tokens[i] << "'" << std::endl;
    }
    std::cout << "Total _tokens: " << _tokens.size() << std::endl;
}

void Parser::parser(){
	_i = 0;
	while (_i < _tokens.size()){
		if (_tokens[_i] == "server") 
		{
			parseServer();
		}
		else {
			throw std::runtime_error("unknow dirrective: " + _tokens[_i]);
		}
		_i++;
	}
}


void Parser::parseServer(){
	Server server;
	server.assigned.assign(2, false);

	std::cout << "Server" << std::endl;
	server.mainRoot = this->_absoluteRootPath;
	_i++;
	if (_tokens[_i] != "{")
		throw std::runtime_error("dirrective \"server\" has no openning \"{\"");
	_i++;
	while (_i < _tokens.size() && _tokens[_i] != "}") {
		parseServerElements(server);
		_i++;
	}
	if (_tokens[_i] != "}")
		throw std::runtime_error("unexpected end of file missing } for server dirrective");
	_servers.push_back(server);

}

void Parser::parseServerElements(Server& server){
	std::string server_tokens[] = {
		"listen",
		"host",
		"server_name",
		"error_page",
		"clientMaxBodySize",
		"location"
	};
	void (Parser::*f[6])(Server&) = {
		&Parser::parseListen,
		&Parser::parseHost,
		&Parser::parseServerName,
		&Parser::parseErrorPage,
		&Parser::parseClientMaxBodySize,
		&Parser::parseRoutes
	};
	for (size_t i = 0; i < 6; ++i) {
		if (_tokens[this->_i] == server_tokens[i]) {
			std::cout << "	" << server_tokens[i]<< std::endl;		
			(this->*f[i])(server);
			return;
		}
	};
	throw std::runtime_error("Error: Unknown directive " + _tokens[this->_i]);
}

void Parser::parseListen(Server& server){

	if (server.assigned[0] == true)
		throw std::runtime_error("port assigned multiple times");
	else
		server.assigned[0] = true;
	std::istringstream iss(_tokens[++_i]);
    iss >> server.port;
    if (iss.fail()) {
        throw std::runtime_error(_tokens[_i] + " not a valid port");
    }
	if (server.port > 65535 || server.port < 0)
        throw std::runtime_error(_tokens[_i] + " not a valid port");

	_i++;
	if (_tokens[_i] != ";")
		throw std::runtime_error("unexpected token" + _tokens[_i]);
}

void Parser::parseHost(Server& server){
    server.host = _tokens[++_i];
	if (!isValidIPv4(server.host))
		throw std::runtime_error("not a valid ipV4 adress: " + _tokens[_i]);
	_i++;
	if (_tokens[_i] != ";")
		throw std::runtime_error("unexpected token" + _tokens[_i]);
}

void Parser::parseServerName(Server& server){
	_i++;
	while (_i < _tokens.size() && _tokens[_i] != ";") {
		server.serverNames.push_back(_tokens[_i]);
		_i++;
	}
}

void Parser::parseErrorPage(Server& server){
	_i++; // Move past "error_page" token
    
    std::vector<int> error_codes;
    std::string error_page_path;
    
    // Parse error codes until we find the path (last token before semicolon)
    while (_i < _tokens.size() && _tokens[_i] != ";") {
        // Check if current token is a number (error code) or path
        std::istringstream iss(_tokens[_i]);
        int error_code;
        
        if (iss >> error_code && iss.eof()) {
            // Successfully parsed as integer - it's an error code
            if (error_code < 100 || error_code > 599) {
                throw std::runtime_error("invalid HTTP error code: " + _tokens[_i]);
            }
            error_codes.push_back(error_code);
        } else {
            // Not a valid integer - assume it's the file path
            error_page_path = _tokens[_i];
            _i++;
            break; // Path should be the last token before semicolon
        }
        _i++;
    }
	if (_tokens[_i] != ";")
		throw std::runtime_error("unexpected token" + _tokens[_i]);
	for (std::vector<int>::iterator i = error_codes.begin(); i < error_codes.end(); i++) {
		server.errorPages[*i] = error_page_path;
	}
}

void Parser::parseClientMaxBodySize(Server& server){
	if (server.assigned[1] == true)
		throw std::runtime_error("clientMaxBodySize assigned multiple times");
	else
		server.assigned[1] = true;
	std::istringstream iss(_tokens[++_i]);
    iss >> server.clientMaxBodySize;
    if (iss.fail()) {
        throw std::runtime_error(_tokens[_i] + " not a valid client max body size");
    }
	if (_tokens[++_i] != ";")
		throw std::runtime_error("unexpected token" + _tokens[_i]);
}

void Parser::parseRoutes(Server& server){
	Route route;
	route.assigned.assign(8, false);
	route.location = _tokens[++_i];
	_i++;
	if (_tokens[_i] != "{")
		throw std::runtime_error("directive \"location\" has no openning \"{\"");
	_i++;
	while (_i < _tokens.size() && _tokens[_i] != "}") {
		parseRouteElements(route);
		_i++;
	}
	if (_tokens[_i] != "}")
		throw std::runtime_error("unexpected end of file missing } for route dirrective");
	server.routes.push_back(route);
}

void Parser::parseRouteElements(Route& route){
	std::string route_tokens[] = {
		"root",
		"index",
		"allowedMethods",
		"autoindex",
		"uploadStore",
		"uploadEnabled",
		"cgiExtension",
		"cgiPath",
	};

	void (Parser::*f[8])(Route&) = {
		&Parser::parseRoot,
		&Parser::parseIndex,
		&Parser::parseAllowMethods,
		&Parser::parseAutoIndex,
		&Parser::parseUploadStore,
		&Parser::parseUploadEnable,
		&Parser::parseCgiExtention,
		&Parser::parseCgiPath,
	};

	for (size_t i = 0; i < 8; ++i) {
		if (_tokens[this->_i] == route_tokens[i]) {
			if (route.assigned[i] == true)
				throw std::runtime_error(_tokens[_i] + " is already assigned in the location " + route.location);
			std::cout << "		" << route_tokens[i]<< std::endl;	
			route.assigned[i] = true;	
			(this->*f[i])(route);
			return;
		}
	};
	throw std::runtime_error("Unknown directive in location: " + _tokens[_i]);
}
void Parser::parseRoot(Route& route){
	route.root = _tokens[++_i];
	if (_tokens[++_i] != ";")
		throw std::runtime_error("unexpected token" + _tokens[_i] + " at root");
}
void Parser::parseIndex(Route& route){
	_i++;
	while (_i < _tokens.size() && _tokens[_i] != ";") {
		route.index.push_back(_tokens[_i]);
		_i++;
	}
}
void Parser::parseAllowMethods(Route& route){
	_i++;
	while (_i < _tokens.size() && _tokens[_i] != ";") {
		if (_tokens[_i] == "GET" || _tokens[_i] == "POST" || _tokens[_i] == "DELETE")
			route.allowedMethods.push_back(_tokens[_i]);
		else
		 throw std::runtime_error("not a valide methode: " + _tokens[_i]);
		_i++;
	}
}
void Parser::parseAutoIndex(Route& route){
	++_i;
	if (_tokens[_i] == "on")
		route.autoindex = true;
	else if (_tokens[_i] == "off")
		route.autoindex = false;
	else {
		throw std::runtime_error("not a valide entry for autoindex: " + _tokens[_i]);
	}
	if (_tokens[++_i] != ";")
		throw std::runtime_error("invalide value " + _tokens[_i] + " for autoindex it must be \"on\" or \"off\"");
}
void Parser::parseUploadStore(Route& route){
	route.uploadStore = _tokens[++_i];
	if (_tokens[++_i] != ";")
		throw std::runtime_error("unexpected token" + _tokens[_i] + " at root");
}
void Parser::parseUploadEnable(Route& route){
	++_i;
	if (_tokens[_i] == "on")
		route.uploadEnabled = true;
	else if (_tokens[_i] == "off")
		route.uploadEnabled  = false;
	else {
		throw std::runtime_error("not a valide entry for autoindex: " + _tokens[_i]);
	}
	if (_tokens[++_i] != ";")
		throw std::runtime_error("invalide value " + _tokens[_i] + " for autoindex it must be \"on\" or \"off\"");
}
void Parser::parseCgiExtention(Route& route){
	_i++;
	while (_i < _tokens.size() && _tokens[_i] != ";") {
		route.cgiExtension.push_back(_tokens[_i]);
		_i++;
	}
}

void Parser::parseCgiPath(Route& route){
	route.cgiPath = _tokens[++_i];
	if (_tokens[++_i] != ";")
		throw std::runtime_error("unexpected token" + _tokens[_i] + " at root");
}

bool Parser::isValidIPv4(const std::string& ip) {
	std::istringstream ss(ip);
	std::string token;
	int parts = 0;

	while (std::getline(ss, token, '.')) {
		if (++parts > 4)
			return false; // trop de parties

		if (token.empty())
			return false;

		// Vérifie que chaque caractère est un chiffre
		for (size_t i = 0; i < token.length(); ++i) {
			if (!isdigit(token[i]))
				return false;
		}

		// Convertir en entier
		int value = std::atoi(token.c_str());
		if (value < 0 || value > 255)
			return false;

		// Empêche les zéros en tête comme "01"
		if (token[0] == '0' && token.length() > 1)
			return false;
	}

	return parts == 4;
}

void Parser::printServer() const{
 for (std::vector<Server>::const_iterator it = _servers.begin(); it != _servers.end(); ++it) {
        it->printServerInfos();
        std::cout << std::endl; // Add spacing between servers
    }
}

std::vector<Server> Parser::getServer() const{
	return _servers;
}

void	Parser::setAbsoluteRootPath(const std::string& path) { this->_absoluteRootPath = path; } // QUENTIN HELP !!! NEEDS TO BE STORED IN EACH SERVER OBJECT (mainRoot) JE LE CTACH AU DEBUT DU MAIN
