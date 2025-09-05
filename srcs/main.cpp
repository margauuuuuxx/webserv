#include "../includes/Parser.hpp"
#include "../includes/Response.hpp"
#include "../includes/Poller.hpp"
#include "../includes/Socket.hpp"
#include "../includes/SocketErray.hpp"
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <csignal>

volatile sig_atomic_t stop = 0; // utilisé pour intercepter SIGINT de manière sûre

std::vector<char> handleRequest(char* buffer, Server& server, int client_fd){

	Response res;
	Request& request = server.requests[client_fd];  // default-constructed if not already there
	if (request.setToParse(buffer))
	{
		request.parse();
		std::cout << "SEND:" << std::endl;
		std::cout << "\e[0;34m" << request.getToParse() << "\e[0m" << std::endl;
		std::cout << "content: " << request.getContent() << std::endl;
		res.handleRequest(request, server);
		request.reset();
		return res.getResponse();
	}

	std::string s = "HTTP/1.1 200 OK\r\nContent-Length: 18\r\n\r\nrequest not complete\n";
	std::vector<char> text(s.begin(), s.end());
	text.push_back('\0');
	return text;
}

void signalHandler(int sig) {
	(void)sig;
	stop  = 1; // change the value of the volatile var
}

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "wrong number of args" << std::endl;
		return 1;
	}

	Parser parser;
	parser.parsefile(argv[1]);
	std::vector<Server> servers = parser.getServer();
	SocketErray sockets;
	std::map<int, Socket*> fdToSocket;
	std::map<int, std::vector<char> > pendingResponses;
	std::map<int, time_t> lastActivity;

	const int TIMEOUT_SECONDS = 10;
	const int POLL_TIMEOUT_MS = 1000; // 1 second poll timeout for periodic cleanup

	try {
		Poller poller;

		// Création des sockets serveurs
		for (size_t i = 0; i < servers.size(); i++) {
			std::cout << "i: " << i << std::endl;
			try {
				sockets.push_back(new Socket(servers[i].port));
				sockets[i]->addServer(servers[i]);
				poller.addFd(sockets[i]->getFd(), POLLIN);
				std::cout << "Serveur en écoute sur le port " << servers[i].port << " ..." << std::endl;
			}
			catch (const std::exception& e) {
				std::cout << "Can't create server on port " << servers[i].port << std::endl;
				std::cout << "because: " << e.what() << std::endl;
			}
		}
		std::cout << "size " << sockets.size() << std::endl;
		if (sockets.size() == 0){
			std::cout << "Couldn't create any server" << std::endl;
			return 1;
		}

		signal(SIGINT, signalHandler);

		while (!stop) {
			// Use timeout for periodic cleanup
			poller.wait(POLL_TIMEOUT_MS);
			std::vector<struct pollfd>& fds = poller.getFds();
			
			// Check for timed out connections BEFORE processing events
			time_t currentTime = time(NULL);
			std::vector<int> timedOutFds;
			
			for (std::map<int, time_t>::iterator it = lastActivity.begin(); it != lastActivity.end(); ++it) {
				int fd = it->first;
				time_t lastTime = it->second;
				
				// Skip server sockets (they don't timeout)
				bool isServerSocket = false;
				for (size_t j = 0; j < sockets.size(); ++j) {
					if (fd == sockets[j]->getFd()) {
						isServerSocket = true;
						break;
					}
				}
				
				if (!isServerSocket && (currentTime - lastTime) > TIMEOUT_SECONDS) {
					timedOutFds.push_back(fd);
				}
			}
			
			// Clean up timed out connections
			for (size_t i = 0; i < timedOutFds.size(); ++i) {
				int fd = timedOutFds[i];
				std::cout << "Client fd=" << fd << " timed out after " << TIMEOUT_SECONDS << " seconds" << std::endl;
				close(fd);
				poller.removeFd(fd);
				fdToSocket.erase(fd);
				pendingResponses.erase(fd);
				lastActivity.erase(fd);
			}

			// Process poll events
			for (size_t i = 0; i < fds.size(); ++i) {
				int fd = fds[i].fd;

				// ---- Nouveaux clients ou données à lire ----
				if (fds[i].revents & POLLIN) {
					bool isListener = false;

					// Vérifie si c'est un socket serveur
					for (size_t j = 0; j < sockets.size(); ++j) {
						if (fd == sockets[j]->getFd()) {
							int client_fd = sockets[j]->clientConnect();
							if (client_fd > 0) {
								std::cout << "Nouveau client connecté (fd=" << client_fd << ")" << std::endl;
								lastActivity[client_fd] = time(NULL);
								poller.addFd(client_fd, POLLIN);
								fdToSocket[client_fd] = sockets[j];
							}
							isListener = true;
							break;
						}
					}

					if (!isListener) {
						char buffer[1024];
						ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);

						if (bytes <= 0) {
							// // Client disconnected or error
							// if (bytes == 0) {
							// 	std::cout << "Client fd=" << fd << " disconnected" << std::endl;
							// } else {
							// 	std::cout << "Error reading from client fd=" << fd << std::endl;
							// }
							// close(fd);
							// poller.removeFd(fd);
							// fdToSocket.erase(fd);
							// pendingResponses.erase(fd);
							// lastActivity.erase(fd);
							continue;
						}

						// Update activity time on successful read
						lastActivity[fd] = time(NULL);

						// Trouver le socket associé
						std::map<int, Socket*>::iterator it = fdToSocket.find(fd);
						if (it != fdToSocket.end()) {
							Socket* sock = it->second;

							buffer[bytes] = '\0';
							std::cout << "===REQUETE===" << std::endl;
							std::cout << "Requête reçue sur port " << sock->getServer().port << std::endl;
							std::cout << "Client fd = " << fd << std::endl;
							std::cout << "Message reçu: " << buffer;

							std::vector<char> response = handleRequest(buffer, sock->getServer(), fd);
							if (!response.empty()) {
								pendingResponses[fd] = response;
								poller.modifyFd(fd, POLLOUT); // passe en écriture
							}
						}
						else {
							std::cout << "Warning: Client fd " << fd << " not found in fdToSocket map" << std::endl;
							close(fd);
							poller.removeFd(fd);
							lastActivity.erase(fd);
						}
					}
				}

				// ---- Données à envoyer ----
				else if (fds[i].revents & POLLOUT) {
					std::map<int, std::vector<char> >::iterator it = pendingResponses.find(fd);
					if (it != pendingResponses.end()) {
						std::vector<char> &data = it->second;
						ssize_t sent = send(fd, &data[0], data.size(), 0);

						if (sent > 0) {
							data.erase(data.begin(), data.begin() + sent); // Supprime la partie envoyée
						} else if (sent < 0) {
							// Send error
							std::cout << "Error sending to client fd=" << fd << std::endl;
							close(fd);
							poller.removeFd(fd);
							fdToSocket.erase(fd);
							pendingResponses.erase(fd);
							lastActivity.erase(fd);
							continue;
						}

						// Si tout est envoyé, retour en lecture
						if (data.empty()) {
							pendingResponses.erase(fd);
							lastActivity[fd] = time(NULL); // Update activity time after successful send
							poller.modifyFd(fd, POLLIN);
						}
					}
				}
				
				// Handle error events
				if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
					std::cout << "Poll error on fd=" << fd << " (revents=" << fds[i].revents << ")" << std::endl;
					close(fd);
					poller.removeFd(fd);
					fdToSocket.erase(fd);
					pendingResponses.erase(fd);
					lastActivity.erase(fd);
				}
			}
		}
	}
	catch (std::exception& e) {
		std::cerr << "Erreur : " << e.what() << std::endl;
	}

	// Cleanup before shutdown
	for (std::map<int, Socket*>::iterator it = fdToSocket.begin(); it != fdToSocket.end(); ++it) {
		close(it->first);
	}
	std::cout << "=== server shutdown ===" << std::endl;
	return 0;
}
