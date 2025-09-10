#include "../includes/includes.hpp"

/*
	This class is responsible for creating a listening socket for a given Server config.
*/

Socket::Socket(int port) {
	listenSocketFd = -1;
	server = NULL;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);//(domaine, type, protocol)
	if (sockfd < 0) {
		throw std::runtime_error("error while creating socket");
	}
	addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
	addrlen = sizeof(addr);
	if (bind(sockfd, (struct sockaddr*)&addr, addrlen) < 0) {
		throw std::runtime_error("error with bind");
	}
	this->backlog = 3;
	if (listen(sockfd, backlog) < 0) {
		throw std::runtime_error("error with listen");
	}
	std::cout << "Socket créé sur le port " << port << " avec fd: " << sockfd << std::endl;
}

Socket::~Socket() {
	close(sockfd);
	if (listenSocketFd != -1) {
		close(listenSocketFd);
	}
	if (server)
		delete server;
}

int Socket::getFd() const{
	return sockfd;
}
int Socket::clientConnect(){

	 if ((listenSocketFd = accept(sockfd, (struct sockaddr*)&(this->clientaddr), (&this->clientaddrlen))) < 0) {
		throw std::runtime_error("error with accept");
    }
	return listenSocketFd;
}

void Socket::addServer(const Server& s){
	if (this->server)
		delete this->server;
	this->server = new Server(s);
}
Server* Socket::getServer(){
	return server;
}
