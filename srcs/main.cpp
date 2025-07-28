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
	std::vector<Socket*> sockets;

	try {
		// Socket sock1(8080);
		// Socket sock2(8181);
		Poller poller;
		for (size_t i = 0; i < servers.size() ; i++) {
			sockets.push_back( new Socket(servers[i].port));
		}

		for (size_t i = 0; i < servers.size() ; i++) {
			sockets[i]->addServer(servers[i]);
			poller.addFd(sockets[i]->getFd(), POLLIN);
			std::cout << "Serveur en écoute sur le port " << servers[i].port << " ..." << std::endl;
		}


		while (true) {
			poller.wait(-1);
			std::vector<struct pollfd>& fds = poller.getFds();

			for (size_t i = 0; i < fds.size(); ++i) {
				int fd = fds[i].fd;

				if (fds[i].revents & POLLIN) {
					bool isListener = false;

					for (size_t j = 0; j < sockets.size(); ++j) {
						if (fd == sockets[j]->getFd()) {
							int client_fd = sockets[j]->clientConnect();
							std::cout << "Nouveau client connecté (fd=" << client_fd << ")" << std::endl;
							poller.addFd(client_fd, POLLIN);
							isListener = true;
							break;
						}
					}

					if (!isListener) {
						char buffer[1024];
						ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);

						if (bytes <= 0) {
							std::cout << "Client déconnecté (fd=" << fd << ")" << std::endl;
							close(fd);
							poller.removeFd(fd);
							i = -1; // recommencer la boucle, car fds a changé
							continue;
						}

						buffer[bytes] = '\0';
						std::cout << "Message reçu: " << buffer;

						std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 18\r\n\r\nHello from server\n";
						send(fd, response.c_str(), response.size(), 0);
					}
				}
			}
		}
	} catch (std::exception& e) {
		std::cerr << "Erreur : " << e.what() << std::endl;
		for (size_t i = 0; i < sockets.size() ; i++) {
			delete sockets[i];
		}
	}

	for (size_t i = 0; i < sockets.size() ; i++) {
		delete sockets[i];
	}
	return 0;
}
