#include "../includes/includes.hpp"

/*
	This class is mostly a data structure.
	It holds all the configuration for a single virtual server.
	--> port, host, error pages & defined routes

	The route struct defines how the server should behave for a specific URL path.
*/

#include "../includes/Server.hpp"


Route::Route() : autoindex(false), uploadEnabled(false) {
	location = "";
	allowedMethods.clear();
	root = "";
	cgiExtension.clear();
	cgiPath = "";
	uploadStore = "";
	path = "";
}

Server::Server(){}
Server::~Server(){}
void Server::printServerInfos() const {
	std::cout << "=== SERVER CONFIGURATION ===" << std::endl;
	std::cout << "Host: " << host << std::endl;
	std::cout << "Port: " << port << std::endl;
	std::cout << "Client Max Body Size: " << clientMaxBodySize << " bytes" << std::endl;

	// Print server names
	std::cout << "Server Names: ";
	if (serverNames.empty()) {
		std::cout << "None";
	} else {
		for (size_t i = 0; i < serverNames.size(); ++i) {
			std::cout << serverNames[i];
			if (i < serverNames.size() - 1) std::cout << ", ";
		}
	}
	std::cout << std::endl;

	// Print error pages
	std::cout << "Error Pages:" << std::endl;
	if (errorPages.empty()) {
		std::cout << "  None defined" << std::endl;
	} else {
		// C++98: Use explicit iterator instead of auto
		for (std::map<int, std::string>::const_iterator it = errorPages.begin(); 
		it != errorPages.end(); ++it) {
			std::cout << "  " << it->first << " -> " << it->second << std::endl;
		}
	}

	// Print routes
	std::cout << "Routes (" << routes.size() << " total):" << std::endl;
	for (size_t i = 0; i < routes.size(); ++i) {
		std::cout << "  Route #" << (i + 1) << ":" << std::endl;

		// Allow methods
		std::cout << "    Allowed Methods: ";
		if (routes[i].allowedMethods.empty()) {
			std::cout << "None";
		} else {
			for (size_t j = 0; j < routes[i].allowedMethods.size(); ++j) {
				std::cout << routes[i].allowedMethods[j];
				if (j < routes[i].allowedMethods.size() - 1) std::cout << ", ";
			}
		}
		std::cout << std::endl;

		// Root directory
		std::cout << "    Root: " << routes[i].root << std::endl;

		// Index files
		std::cout << "    Index Files: ";
		if (routes[i].index.empty()) {
			std::cout << "None";
		} else {
			for (size_t j = 0; j < routes[i].index.size(); ++j) {
				std::cout << routes[i].index[j];
				if (j < routes[i].index.size() - 1) std::cout << ", ";
			}
		}
		std::cout << std::endl;

		// Autoindex
		std::cout << "    Autoindex: " << (routes[i].autoindex ? "Enabled" : "Disabled") << std::endl;

		// HTTP redirects

		// CGI configurations
		std::cout << "    CGI Configurations: ";
		if (routes[i].cgiPath.empty()) {
			std::cout << "None" << std::endl;
		} else {
			std::cout << std::endl;
			std::cout << "cgiPath: " << routes[i].cgiPath << std::endl;
			std::cout << "cgi_extentions: " << routes[i].cgiExtension[0] << std::endl;
			}
		

		// Upload settings
		std::cout << "    Upload Enabled: " << (routes[i].uploadEnabled ? "Yes" : "No") << std::endl;
		if (routes[i].uploadEnabled) {
			std::cout << "    Upload Store: " << routes[i].uploadStore << std::endl;
		}

		if (i < routes.size() - 1) {
			std::cout << std::endl; // Add spacing between routes
		}
	}
	std::cout << "=========================" << std::endl;
}
