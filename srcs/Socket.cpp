#include "../includes/Socket.hpp"

Socket::Socket(int port) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);//(domaine, type, protocol)
	if (sockfd < 0) {
		throw std::runtime_error("error while creating socket");
	}
	addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
	addrlen = sizeof(addr);
	if (bind(sockfd, (struct sockaddr*)&addr, addrlen)) {
		throw std::runtime_error("error with bind");
	}
	backlog = 3;
	if (listen(sockfd, backlog) < 0) {
		throw std::runtime_error("error with listen");
	}
}

Socket::~Socket() {
	close(sockfd);
	close(listenSocketFd);
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
