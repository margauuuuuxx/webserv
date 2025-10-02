#include "../includes/includes.hpp"
#include "Response.hpp" // Make sure Response class is fully defined

volatile sig_atomic_t stop = 0;

// This function is for simple, fully-formed requests.
std::vector<char> handleRequest(Request& request, Server& server){
	Response res;
	res.handleRequest(request, server);
	return (res.getResponse());
}

void signalHandler(int sig) {
	(void)sig;
	stop  = 1;
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
	std::map<int, std::vector<char> > pendingResponses;
	std::map<int, Response> chunkingResponses;

	try {
		Poller poller;

		for (size_t i = 0; i < servers.size(); i++) {
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
						std::vector<char> buffer(MAX_REQUEST_SIZE);
						ssize_t bytes = recv(fd, &buffer[0], buffer.size(), 0);

						if (bytes <= 0) {
							std::cout << "Client déconnecté (fd=" << fd << ")" << std::endl;
							close(fd);
							poller.removeFd(fd);
							fdToSocket.erase(fd);
							pendingResponses.erase(fd);
							chunkingResponses.erase(fd);
							continue;
						}

						std::map<int, Socket*>::iterator it = fdToSocket.find(fd);
						if (it != fdToSocket.end()) {
							Socket* sock = it->second;
							Server* server = sock->getServer();
							Request& request = server->requests[fd];
							request.setClientIP(sock->getClientIP());
							request.appendToRawRequest(&buffer[0], bytes);
							request.parse(server->clientMaxBodySize);

							if (request.parsingFinished()) {
								Response& res = chunkingResponses[fd];
								res.handleRequest(request, *server);

								if (res.isChunkingActive())
									poller.modifyFd(fd, POLLOUT);
								else {
									std::vector<char> responseData = res.getResponse();
									if (!responseData.empty()) {
										pendingResponses[fd] = responseData;
										poller.modifyFd(fd, POLLOUT);
									}
									chunkingResponses.erase(fd);
								}
								request.reset();
							} else if (request.parsingError()) {
								int code = request.getErrorCode();
								const std::string& message = request.getStatusMessage();
								std::vector<char> response = generateErrorResponse(code, message);
								pendingResponses[fd] = response;
								poller.modifyFd(fd, POLLOUT);
								request.reset();
							}
						}
					}
				} else if (fds[i].revents & POLLOUT) {
					std::map<int, Response>::iterator it_chunk = chunkingResponses.find(fd);
					if (it_chunk != chunkingResponses.end()) {
						Response& res = it_chunk->second;
						
						if (res.getResponseBuffer().empty() && res.isChunkingActive()) {
							res.prepareNextChunk(4096);
						}

						const std::vector<char>& buffer = res.getResponseBuffer();
						if (!buffer.empty()) {
							ssize_t sent = send(fd, &buffer[0], buffer.size(), 0);
							if (sent > 0) {
								res.consumeBufferBytes(sent);
							}
						}

						if (!res.isChunkingActive() && res.getResponseBuffer().empty()) {
							chunkingResponses.erase(it_chunk);
							poller.modifyFd(fd, POLLIN);
						}
					} else {
						std::map<int, std::vector<char> >::iterator it = pendingResponses.find(fd);
						if (it != pendingResponses.end()) {
							std::vector<char> &data = it->second;
							ssize_t sent = send(fd, &data[0], data.size(), 0);

							if (sent > 0) {
								data.erase(data.begin(), data.begin() + sent);
							}

							if (data.empty()) {
								pendingResponses.erase(it);
								poller.modifyFd(fd, POLLIN);
							}
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