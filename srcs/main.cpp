#include "../includes/Parser.hpp"
#include "../includes/Poller.hpp"
#include "../includes/Socket.hpp"
#include <cstddef>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <vector>

int main() {
	Parser parser;
	parser.parsefile("simplest_config.conf");
	std::vector<Server> servers = parser.getServer();
	std::vector<Socket> sockets;

	try {
		Poller poller;

		for (size_t i = 0; i < servers.size() ; i++) {
			Socket sock(servers[i].port);
			sock.addServer(servers[i]);
			sockets.push_back(sock);
			poller.addFd(sock.getFd(), POLLIN);
			std::cout << "Serveur en écoute sur le port " << servers[i].port << " ..." << std::endl;
			std::cout << "fd: " << sock.getFd() << std::endl;
		}


	// 	while (true) {
	// 		poller.wait(-1);
	//
	// 		std::vector<struct pollfd>& fds = poller.getFds();
	// 		for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) {
	// 			if (it->revents & POLLIN) {
	// 				bool isListener = false;
	//
	// 				for (size_t i = 0; i < sockets.size(); i++) {
	// 					if (it->fd == sockets[i].getFd()) {
	// 						int client_fd = sockets[i].clientConnect();
	// 						std::cout << "Nouveau client connecté (fd=" << client_fd << ")" << std::endl;
	// 						poller.addFd(client_fd, POLLIN);
	// 						isListener = true;
	// 						break;
	// 					}
	// 				}
	//
	// 				if (!isListener) {
	// 					char buffer[1024];
	// 					ssize_t bytes_read = read(it->fd, buffer, 1023);
	//
	// 					if (bytes_read <= 0) {
	// 						std::cout << "Client déconnecté (fd=" << it->fd << ")" << std::endl;
	// 						poller.removeFd(it->fd);
	// 						break;
	// 					}
	//
	// 					buffer[bytes_read] = '\0';
	// 					std::cout << "Message reçu: " << buffer;
	//
	// 					std::string response = "Hello from server\n";
	// 					send(it->fd, response.c_str(), response.size(), 0);
	// 				}
	// 			}
	//
	// 		}
	// 	}
	} catch (std::exception& e) {
		std::cerr << "Erreur : " << e.what() << std::endl;
	}

	return 0;
}
