#include "../includes/includes.hpp"

volatile sig_atomic_t stop = 0; // utilisé pour intercepter SIGINT de manière sûre

std::vector<char> handleRequest(char* buffer, Server& server, int client_fd, int bytes){

	Response res;
	Request& request = server.requests[client_fd];  // default-constructed if not already there
	//std::cout << "buffer dans handleRequest: " << buffer << std::endl;
	//std::cout << "on va dans setToParse" << std::endl;
	if (request.setToParse(buffer, bytes))
	{
		//std::cout << "setToParse OK" << std::endl;
		//std::cout << "check de toParse: " << std::endl << request.getToParse() << std::endl;
		request.parse();
		std::cout << "\e[0;31mBody:\e[0;m" << std::endl;
		std::cout.write(request.getBody(), bytes) << std::endl;
		std::cout << "SEND:" << std::endl;
		//std::cout << "\e[0;34m" << request.getToParse() << "\e[0m" << std::endl;
		res.handleRequest(request, server);
		request.reset();
		return res.getResponse();
	}

	std::vector<char> text;
	return text;
}

void signalHandler(int sig) {
	(void)sig;
	stop  = 1; // change the value of the volatile var
}

int main(int argc, char **argv) {
	if (argc != 2) {
		return std::cout << RED << "Error: " << RESET << "wrong number of args" << std::endl, 1;	
	}

	Parser parser;
	parser.parsefile(argv[1]);
	std::vector<Server> servers = parser.getServer();
	SocketArray sockets;
	std::map<int, Socket*> fdToSocket;
	std::map<int, std::vector<char> > pendingResponses; // stocke les réponses en attente

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
			poller.wait(-1);
			std::vector<struct pollfd>& fds = poller.getFds();

			for (size_t i = 0; i < fds.size(); ++i) {
				int fd = fds[i].fd;

				// ---- Nouveaux clients ou données à lire ----
				if (fds[i].revents & POLLIN) {
					bool isListener = false;

					// Vérifie si c'est un socket serveur
					for (size_t j = 0; j < sockets.size(); ++j) {
						if (fd == sockets[j]->getFd()) {
							int client_fd = sockets[j]->clientConnect();
							std::cout << "Nouveau client connecté (fd=" << client_fd << ")" << std::endl;
							poller.addFd(client_fd, POLLIN);
							fdToSocket[client_fd] = sockets[j];
							isListener = true;
							break;
						}
					}

					if (!isListener) {
						std::vector<char> buffer(MAX_REQUEST_SIZE);
						ssize_t bytes = recv(fd, &buffer[0], buffer.size(), 0); // last parameter = flags

						if (bytes <= 0) {
							std::cout << "Client déconnecté (fd=" << fd << ")" << std::endl;
							close(fd);
							poller.removeFd(fd);
							fdToSocket.erase(fd);
							pendingResponses.erase(fd);
							continue;
						}

						// Trouver le socket associé
						std::map<int, Socket*>::iterator it = fdToSocket.find(fd);
						if (it != fdToSocket.end()) {
							Socket* sock = it->second;
							Server* server = sock->getServer();
							Request& request = server->requests[fd];
							request.setClientIP(sock->getClientIP());

							// buffer[bytes] = '\0';
							std::cout << "===REQUETE===" << std::endl;
							std::cout << "Requête reçue sur port " << sock->getServer()->port << std::endl;
							std::cout << "Client fd = " << fd << std::endl;
							std::cout << "Message reçu: ";
							std::cout.write(&buffer[0], bytes);
							//std::cout << buffer << std::endl;
							//std::cout << "bytes: " << bytes << std::endl;
							std::cout << std::endl;

							std::vector<char> response = handleRequest(&buffer[0], *(sock->getServer()), fd, bytes);
							if (!response.empty()) {
								pendingResponses[fd] = response;
								poller.modifyFd(fd, POLLOUT); // passe en écriture
							}
						}
						else {
							throw std::runtime_error("Client fd non trouvé lors de la réception");
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
						}

						// Si tout est envoyé, retour en lecture
						if (data.empty()) {
							pendingResponses.erase(fd);
							poller.modifyFd(fd, POLLIN);
						}
					}
				}
			}
		}
	}
	catch (std::exception& e) {
		std::cerr << "Erreur : " << e.what() << std::endl;
	}

	std::cout << "=== server shutdown ===" << std::endl;
	return 0;
}
