#pragma once

#include "includes.hpp"

class Request
{
private:
	// sockaddr_in	_clientAddress;
	int			_newSocket;
	// socklen_t	_clientAddrlen;
	std::string	_content;
	std::string _toParse;
	bool		_transferEncoding;
	bool		_waitingForData;
	size_t		_contentLen;
	size_t		_contentLenCopy;
	size_t		_appendLen;
public:
	Request(void);
	~Request(void);

	// int acceptRequest(int sockfd);
	int acceptRequest(int bytesRead, char buffer[MAX_REQUEST_SIZE], int clientFd);
	std::string const &getContent(void) const;
	std::string const &getToParse(void) const;
	size_t const &getContentLen(void) const;
	size_t const &getContentLenCopy(void) const;
	bool const &getTransferEncoding(void) const;
	bool const &getWaitingState(void) const;
	int	getSocket(void) const;
	int setToParse(char buffer[MAX_REQUEST_SIZE]);
	void setContentLen(size_t len);
	void decrementLen(void);
	void setTransferEncoding(bool state);
	void closeSocket(void);
};
