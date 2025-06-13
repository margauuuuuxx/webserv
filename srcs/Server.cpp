#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"

WebServer::WebServer(void): _port(8080) {}

WebServer::~WebServer(void) {}

void	WebServer::setup(void)
{
	int reuse = 1;
	socklen_t	addrlen = sizeof(this->_address);

	if ((this->_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Socket failed");
	memset(&this->_address, 0, sizeof(this->_address));
	this->_address.sin_family = PF_INET;
	this->_address.sin_port = htons(this->_port);
	this->_address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (setsockopt(this->_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
		throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");

	if (setsockopt(this->_sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
		throw std::runtime_error("setsockopt(SO_REUSEPORT) failed");


	if (bind(this->_sockfd, (struct sockaddr *)&this->_address, addrlen) < 0)
		throw std::runtime_error("Bind failed");
	if (listen(this->_sockfd, 1) < 0)
		throw std::runtime_error("Listen failed");
}

std::string getPage(std::string page)
{
	std::ifstream infile;
	std::string res;
	std::string content;
	std::string line;

	res.clear();
	if (page.empty())
	{
		res.append("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
		return (res);
	}
	infile.open(page.c_str());
	if (!infile.is_open())
	{
		res.append("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
		return (res);
	}
	while (getline(infile, line))
	{
		content.append(line);
	}
	res.append("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: ");
	char *number = ft_itoa(content.length());
	delete[] number;
	res.append("\r\n\r\n");
	res.append(content);
	return (res);
}

void WebServer::run()
{
	Request	request;
	std::string	hello;

	while (1)
	{
		if (!request.acceptRequest(this->_sockfd))
			continue ;
		hello = getPage(request.getContent());
		if (hello.empty())
		{
			std::cerr << "Content does not exist" << std::endl;
			continue ;
		}
		send(request.getSocket() , hello.c_str() , hello.length(), 0);
	}
	request.closeSocket();
}
