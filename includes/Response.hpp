#pragma once
#include <string>
#include <vector>
#include "Server.hpp"
#include "Request.hpp"

struct location{
	Route *route;
	std::string filename;
};
class Response{
public:
	Response();
	~Response();
	void handleRequest(Request& req, Server& server);
	void handleGET(Request& req, Server& server, struct location& location);
	std::vector<char> getResponse();
	bool startsWith(const std::string &str, const std::string &prefix);
	struct location getLocationAndFilename(Request& req, Server& server);
	void setContentType(std::string file);
	std::string getHTTPDate();
	std::string generateDirectoryListing(const std::string &path, const std::string &uri);
	void handleGetCgi(std::string filename, std::string args);
private:
	std::vector<char> _content;
	std::string _content_type;
	std::string _http_version;
	size_t _content_size;
	size_t _status_code;
};
