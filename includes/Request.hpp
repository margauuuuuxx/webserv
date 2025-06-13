#pragma once

#include "webserv.hpp"

class Request
{
private:
	sockaddr_in	_client_address;
	int			_new_socket;
	socklen_t	_client_addrlen;
	std::string	_content;
public:
	Request(void);
	~Request(void);

	int acceptRequest(int sockfd);
	std::string const &getContent(void) const;
	int	getSocket(void) const;
	void closeSocket(void);
};
