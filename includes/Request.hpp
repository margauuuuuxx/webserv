#pragma once

#include "includes.hpp"

class Request
{
private:
	Request(const Request &other);
	Request& operator=(const Request &other);
	std::string	_toParse;
	bool		_error;
	bool		_transferEncoding;
	bool		_waitingForData;
	size_t		_contentLen;
	size_t		_contentLenCopy;
	size_t		_appendLen;
	std::string	_method;
	std::string	_content;
	std::string	_version;
	std::map <std::string, std::string> _headers;
	std::multimap<std::string, std::string> _multiHeaders;
	std::string _body;
public:
	Request(void);
	~Request(void);

	int acceptRequest(int bytesRead, char buffer[MAX_REQUEST_SIZE], int clientFd);
	std::string	const	&getMethod(void) const;
	std::string	const	&getContent(void) const;
	std::string	const	&getVersion(void) const;
	std::map<std::string, std::string>	const	&getHeaders(void) const;
	std::string	const	&getBody(void) const;
	std::string	const	&getToParse(void) const;
	size_t		const	&getContentLen(void) const;
	size_t		const	&getContentLenCopy(void) const;
	bool		const	&getTransferEncoding(void) const;
	bool		const	&getWaitingState(void) const;
	int					getSocket(void) const;
	bool		const	getErrorFlag(void) const;
	int					assignError(std::string error);
	int					setToParse(char buffer[MAX_REQUEST_SIZE]);
	void				setContentLen(size_t len);
	void				setTransferEncoding(bool state);
	void				parse(void);
	void				reset(void);
};
