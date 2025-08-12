#pragma once

#include "includes.hpp"

class Response {
public:
	Response();
	~Response();

	void		handleRequest(Request& req, Server& server);
	void		handleGET(Request& req, Server& server);
	void		handlePOST(Request& req, Server& server);
	void		handleDELETE(Request& req, Server& server);
	std::string	getResponse();

private:
	std::string	_content;
	std::string	_httpVersion;
	size_t		_contentSize;
	size_t		_statusCode;
};
