#include "../includes/Response.hpp"

Response::Response(const Request& req): _req(req) {
	if (req.getErrorFlag())
		_response = _req.getToParse();
	else
	{
		std::string method = _req.getMethod();
		if (method == "GET")
		{
			//GET
		}
		else if (method == "POST")
		{
			//POST
		}
		else if (method == "DELETE")
		{
			//DELETE
		}
	}
}

Response::~Response() {}

std::string const &Response::getResponse() const{
	return (this->_response);
}