#pragma once

#include "includes.hpp"

class Response{
public:
	Response();
	~Response();
	void handleRequest(Request& req, Server& server);
	void handleGET(Request& req, Server& server);
	std::string getResponse();
private:
	std::string _content;
	std::string _http_version;
	size_t _content_size;
	size_t _status_code;
};
