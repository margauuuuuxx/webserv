#include "../includes/includes.hpp"

volatile sig_atomic_t stop = 0; // utilisé pour intercepter SIGINT de manière sûre

std::string handleRequest(char* buffer, Server& server, int client_fd){

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
		// return "HTTP/1.1 200 OK\r\nContent-Length: 17\r\n\r\nrequest complete\n";
		//clients.erase(clientFd);
		// request.acceptRequest(bytesRead, buffer, clientFd);
		// hello = getPage(request.getContent());
		// send(clientFd , hello.c_str() , hello.length(), 0);
	}
	return "HTTP/1.1 200 OK\r\nContent-Length: 18\r\n\r\nrequest not complete\n";
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
	srand(time(NULL)); // for upload ame files generation

	try {
		Poller poller;
		for (size_t i = 0; i < servers.size() ; i++) {
			try {
				Socket* newSocket = new Socket(servers[i].port);
				newSocket->addServer(servers[i]);
				poller.addFd(newSocket->getFd(), POLLIN);
				sockets.pushBack(newSocket);
				std::cout << "Serveur en écoute sur le port " << servers[i].port << " ..." << std::endl;
			}
			catch (const std::exception& e) {
				std::cout << "Can\'t create serveur on port " << servers[i].port << std::endl;
				std::cout << "because: " << e.what() << std::endl;
			}
		}

		//va faloir gerer les signaux => peut-etre utuliser un pipe || volatile variable
		signal(SIGINT, signalHandler);
		while (!stop) {
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
							fdToSocket[client_fd] = sockets[j];
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

						//find the correspondant socket to parse the request based on this server and socket
						std::map<int, Socket*>::iterator it = fdToSocket.find(fd);
						if (it != fdToSocket.end()) {
							Socket* sock = it->second;
							Server* server = sock->getServer();
							Request& request = server->requests[fd];
							request.setClientIP(sock->getClientIP());

							std::cout << "===REQUETE===" << std::endl;
							std::cout << "Requête reçue sur socket liée au port " << sock->getServer()->port << std::endl;
							std::cout << "Client fd = " << fd << std::endl;
							buffer[bytes] = '\0';
							std::cout << "Message reçu: " << buffer;
							std::string response = handleRequest(buffer, *sock->getServer(), fd);
							if (!response.empty())
								send(fd, response.c_str(), response.size(), 0);
						}
						else {
							throw std::runtime_error("didn\'t find the client fd when receved the request");
						}
					}
				}
			}
		}
	} catch (std::exception& e) {
		std::cerr << "Erreur : " << e.what() << std::endl;
	}
	std::cout << "===server shutdown===" << std::endl;
	return 0;
}
