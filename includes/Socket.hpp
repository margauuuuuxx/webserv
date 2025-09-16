#pragma once

#include "includes.hpp"

class Server;

class Socket {
public:
	Socket(int port);
	~Socket();

	int			getFd() const;
	Server*		getServer();
	int 		clientConnect();
	void		addServer(const Server& s);
	std::string	getClientIP() const;

private:
	int 		sockfd;
	int 		listenSocketFd;
	sockaddr_in	addr;
	sockaddr_in clientaddr;
	socklen_t	addrlen;
	socklen_t	clientaddrlen;
	int			backlog;
	Server*		server;
};
