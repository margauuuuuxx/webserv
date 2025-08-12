#include "../includes/includes.hpp"

void Server::printServerInfos() const {
	std::cout << "=== SERVER CONFIGURATION ===" << std::endl;
	std::cout << "Host: " << host << std::endl;
	std::cout << "Port: " << port << std::endl;
	std::cout << "Client Max Body Size: " << client_max_body_size << " bytes" << std::endl;

	// Print server names
	std::cout << "Server Names: ";
	if (server_names.empty()) {
		std::cout << "None";
	} else {
		for (size_t i = 0; i < server_names.size(); ++i) {
			std::cout << server_names[i];
			if (i < server_names.size() - 1) std::cout << ", ";
		}
	}
	std::cout << std::endl;

	// Print error pages
	std::cout << "Error Pages:" << std::endl;
	if (error_pages.empty()) {
		std::cout << "  None defined" << std::endl;
	} else {
		// C++98: Use explicit iterator instead of auto
		for (std::map<int, std::string>::const_iterator it = error_pages.begin(); 
		it != error_pages.end(); ++it) {
			std::cout << "  " << it->first << " -> " << it->second << std::endl;
		}
	}

	// Print routes
	std::cout << "Routes (" << routes.size() << " total):" << std::endl;
	for (size_t i = 0; i < routes.size(); ++i) {
		std::cout << "  Route #" << (i + 1) << ":" << std::endl;

		// Allow methods
		std::cout << "    Allowed Methods: ";
		if (routes[i].allow_methods.empty()) {
			std::cout << "None";
		} else {
			for (size_t j = 0; j < routes[i].allow_methods.size(); ++j) {
				std::cout << routes[i].allow_methods[j];
				if (j < routes[i].allow_methods.size() - 1) std::cout << ", ";
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
		if (routes[i].cgi_path.empty()) {
			std::cout << "None" << std::endl;
		} else {
			std::cout << std::endl;
			std::cout << "cgi_path: " << routes[i].cgi_path << std::endl;
			std::cout << "cgi_extentions: " << routes[i].cgi_extension[0] << std::endl;
			}
		

		// Upload settings
		std::cout << "    Upload Enabled: " << (routes[i].upload_enable ? "Yes" : "No") << std::endl;
		if (routes[i].upload_enable) {
			std::cout << "    Upload Store: " << routes[i].upload_store << std::endl;
		}

		if (i < routes.size() - 1) {
			std::cout << std::endl; // Add spacing between routes
		}
	}
	std::cout << "=========================" << std::endl;
}
