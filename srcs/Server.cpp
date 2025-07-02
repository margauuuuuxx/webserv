#include "../includes/Server.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"

extern int gSigInt;

Server::Server(void): _port(8080) {}

Server::~Server(void) {}

void	Server::setup(void)
{
	int reuse = 1;
	socklen_t	addrlen = sizeof(this->_address);

	if ((this->_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Socket failed");
	memset(&this->_address, 0, sizeof(this->_address));
	this->_address.sin_family = PF_INET;
	this->_address.sin_port = htons(this->_port);
	this->_address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (setsockopt(this->_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
		throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");

	if (setsockopt(this->_sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
		throw std::runtime_error("setsockopt(SO_REUSEPORT) failed");


	if (bind(this->_sockfd, (struct sockaddr *)&this->_address, addrlen) < 0)
		throw std::runtime_error("Bind failed");
	if (listen(this->_sockfd, 1) < 0)
		throw std::runtime_error("Listen failed");
}

std::string getPage(std::string page)
{
	std::ifstream infile;
	std::string res;
	std::string content;
	std::string line;

	res.clear();
	if (page.empty())
	{
		res.append("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
		return (res);
	}
	infile.open(page.c_str());
	if (!infile.is_open())
	{
		res.append("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
		return (res);
	}
	while (getline(infile, line))
	{
		content.append(line);
	}
	res.append("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: ");
	char *number = ft_itoa(content.length());
	delete[] number;
	res.append("\r\n\r\n");
	res.append(content);
	return (res);
}

void Server::run()
{
	Request	request;
	std::string	hello;
	std::map <int, Request> clients;
	std::map <int, std::time_t> timeout;

	/*
		serverFd est la socket d'écoute, toutes les requêtes passent par elle.
		Le programme la stocke dans _pollFd (vecteur de structure pollfd) en premier.
		.fd = fd de la socket
		.events = l'evenement recherché, ici POLLIN représente l'entré (input)
		.revents = mis à 0 par défaut et est mis à jour quand il se passe quelque chose sur la socket
	*/
	struct pollfd serverFd;

	// std::cout << "into run" << std::endl;
	serverFd.fd = this->_sockfd;
	serverFd.events = POLLIN;
	serverFd.revents = 0;
	this->_pollFd.push_back(serverFd);
	int clientSocket;
	while (1)
	{
		/*
			Appel à poll() et début d'une boucle pour itérer dans le vecteur _PollFd
			pour checker chaque socket.
		*/
		if ((poll(this->_pollFd.data(), this->_pollFd.size(), 1000)) < 0)
			throw std::runtime_error("Poll failed");
		// std::cout << "poll ok" << std::endl;
		for (size_t i = 0; i < this->_pollFd.size(); ++i)
		{
			// if (!request.acceptRequest(this->_sockfd))
			// 	continue ;

			std::time_t now = std::time(NULL);
			for (size_t i = 0; i < timeout.size(); i++)
			{
				int clientTimeout = this->_pollFd[i].fd;
				// std::cout << "check du: " << clientTimeout << std::endl;
				if ((timeout[clientTimeout] + 10 < now && !(clients[clientTimeout].getToParse()).empty())
					|| (timeout[clientTimeout] + 10 < now && clients[clientTimeout].getContentLenCopy() > 0 && clients[clientTimeout].getContentLenCopy() != std::string::npos))
				{
					std::cout << "timeout: " << timeout[i] << "/" << now << std::endl;
					std::string error = makeError(408, "Request Timeout");
					send(clientTimeout, error.c_str(), error.size(), 0);
					std::cout << "SEND: " << error << std::endl;
					clients.erase(clientTimeout);
					close(clientTimeout);
					this->_pollFd.erase(this->_pollFd.begin() + i);
					std::cout << "client " << i << " deconnecté" << std::endl;
				}
			}

			if (this->_pollFd[i].revents)
			{
				if (this->_pollFd[i].fd == serverFd.fd)
				{
					/*
						Si i = 0, et donc pollFd[i] == serverFd, on a reçu une nouvelle requête.
						Le programme l'accepte, crée une structure pollfd adéquate et push dans le vecteur.
					*/
					if ((clientSocket = accept(this->_sockfd, NULL, NULL)) < 0)
						throw std::runtime_error("accept failed");
					std::cout << "Client connecté" << std::endl;
					struct pollfd newClient;
					newClient.fd = clientSocket;
					newClient.events = POLLIN;
					newClient.revents = 0;
					this->_pollFd.push_back(newClient);
				}
				else if (this->_pollFd[i].revents & POLLIN)
				{
					/*
						Cette condition dit "nouvelle input sur l'actuel pollfd"
						Le programme recoit l'input, 
							si bytesRead < 0: Erreur
							si bytesRead == 0: Client deconnecté
							sinon: reception de la requête bonne, envoie des infos nécessaire
					*/
					int clientFd = this->_pollFd[i].fd;
					int bytesRead = 0;
					char buffer[MAX_REQUEST_SIZE] = {0};

					bytesRead = recv(this->_pollFd[i].fd, buffer, MAX_REQUEST_SIZE, 0);
					if (bytesRead < 0)
					{
						clients.erase(clientFd);
						close(clientFd);
						this->_pollFd.erase(this->_pollFd.begin() + i);
						--i;
						std::cout << "Error recv" << std::endl;
					}
					else if (bytesRead == 0 || this->_pollFd[i].revents & (POLLHUP | POLLERR))
					{
						if (bytesRead > 0 && clients[clientFd].getContentLenCopy() > 0)
						{
							std::string error = makeError(400, "Bad Request");
							send(clientFd, error.c_str(), error.size(), 0);
							std::cout << "SEND" << error << std::endl;
						}
						clients.erase(clientFd);
						close(clientFd);
						this->_pollFd.erase(this->_pollFd.begin() + i);
						--i;
						std::cout << "client " << i << " deconnecté" << std::endl;
					}
					else if (bytesRead > MAX_REQUEST_SIZE)
					{
						std::string error = makeError(413, "Content Too Large");
						send(clientFd, error.c_str(), error.size(), 0);
						clients[clientFd].reset();
						clients.erase(clientFd);
					}
					else
					{
						timeout[clientFd] = std::time(NULL);
						if (clients[clientFd].setToParse(buffer))
						{
							clients[clientFd].parse();
							std::cout << "SEND:" << std::endl;
							std::cout << "\e[0;34m" << clients[clientFd].getToParse() << "\e[0m" << std::endl;
							clients[clientFd].reset();
							clients.erase(clientFd);
							// request.acceptRequest(bytesRead, buffer, clientFd);
							// hello = getPage(request.getContent());
							// send(clientFd , hello.c_str() , hello.length(), 0);
						}
					}
				}
			}
		}
	}
	// request.closeSocket();
}
