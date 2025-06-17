#pragma once

#include "includes.hpp"

class Request
{
private:
	sockaddr_in	_clientAddress;
	int			_newSocket;
	socklen_t	_clientAddrlen;
	std::string	_content;
public:
	Request(void);
	~Request(void);

	// int acceptRequest(int sockfd);
	int acceptRequest(int bytesRead, char buffer[MAX_REQUEST_SIZE], int clientFd);
	std::string const &getContent(void) const;
	int	getSocket(void) const;
	void closeSocket(void);
};
