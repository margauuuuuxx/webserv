#include "../includes/Parser.hpp"
#include "../includes/Poller.hpp"
#include "../includes/Socket.hpp"
#include <cstddef>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <map>
#include <vector>

int main() {
	Parser parser;
	parser.parsefile("config.conf");
	std::vector<Server> servers = parser.getServer();
	std::vector<Socket> sockets;

    try {
        Socket server1(8080);
        // Socket server2(8080);
        Poller poller;

        poller.addFd(server1.getFd(), POLLIN);
        // poller.addFd(server2.getFd(), POLLIN);

        std::cout << "Serveur en écoute sur le port 8080..." << std::endl;

        while (true) {
            poller.wait(-1);

            std::vector<struct pollfd>& fds = poller.getFds();
            for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) {
                if (it->revents & POLLIN) {
                    if (it->fd == server1.getFd()) {
                        int client_fd = server1.clientConnect();
                        std::cout << "Nouveau client connecté (fd=" << client_fd << ")" << std::endl;
                        poller.addFd(client_fd, POLLIN);
                        break; // important pour éviter l'itération sur un vecteur modifié
                    } else {
                        char buffer[1024];
                        ssize_t bytes_read = read(it->fd, buffer, 1023);

                        if (bytes_read <= 0) {
                            std::cout << "Client déconnecté (fd=" << it->fd << ")" << std::endl;
                            poller.removeFd(it->fd);
                            break;
                        }

                        buffer[bytes_read] = '\0';
                        std::cout << "Message reçu: " << buffer;

                        std::string response = "Hello from server\n";
                        send(it->fd, response.c_str(), response.size(), 0);
                    }
                }
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }

    return 0;
}
