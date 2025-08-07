#pragma once
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "Server.hpp"

class Socket {
public:
	Socket(int port);
	~Socket();
	int getFd() const;
	Server& getServer();
	int clientConnect();
	void addServer(const Server& s);

private:
	int sockfd;
	int listenSocketFd;
	sockaddr_in addr;
	sockaddr_in clientaddr;
	socklen_t addrlen;
	socklen_t clientaddrlen;
	int backlog;
	Server server;
	
};
