#include "Request.hpp"

Request::Request(void): _client_addrlen(sizeof(_client_address)) {}
Request::~Request(void) {}

std::string makeError(int code, std::string message)
{
	std::ostringstream oss;

	oss << "HTTP/1.1 " << code << " " << message << "\r\n"
		<< "Content-Length: 0\r\n"
		<< "Connection: close\r\n"
		<< "\r\n";
	return (oss.str());
}

int isRawEmpty(std::string raw)
{
	size_t count = 0;

	for (std::string::iterator it = raw.begin(); it != raw.end(); it++)
	{
		if (*it == '\r' || *it == '\n' || *it == ' ')
			count++;
		if (*it == '\r')
			std::cout << "r" << std::endl;
		else if (*it == '\n')
			std::cout << "n" << std::endl;
		else if (*it == ' ')
			std::cout << " " << std::endl;
		else
			std::cout << (int)*it << std::endl;
	}
	std::cout << "count" << count << std::endl;
	std::cout << "raw.length()" <<raw.length() << std::endl;
	if (count == raw.length())
		return (1);
	return (0);
}

void parseRequest(std::string raw, int sockfd)
{
	std::istringstream iss(raw);
	std::string method;
	std::string path;
	std::string version;
	std::ifstream test;

	if (isRawEmpty(raw))
		return ;
	iss >> method >> path >> version;
	std::cout << "méthode: " << method << std::endl;
	std::cout << "chemin: " << path << std::endl;
	std::cout << "version: " << version << std::endl;
	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		std::string error = makeError(405, "Method Not Allowed");
		send(sockfd, error.c_str(), error.length(), 0);
		// throw std::runtime_error("mauvaise méthode");
		close(sockfd);
		return ;
	}
	if (version != "HTTP/1.1")
	{
		std::cerr << "probleme version" << std::endl;
		throw std::runtime_error("mauvaise version");
	}
	std::string file = path.substr(1);
	if (!file.empty())
	{
		test.open(file.c_str());
		if (!test.is_open())
		{
			std::cerr << "probleme page" << std::endl;
			throw std::runtime_error("mauvaise page");
		}
		test.close();
	}
}

std::string whichPage(char buffer[MAX_REQUEST_SIZE])
{
	int i = 0;
	std::string res;

	std::cout << buffer << std::endl;
	while (buffer[i])
	{
		if (buffer[i] == '/' && buffer[i + 1])
		{
			i++;
			break ;
		}
		i++;
	}
	while (buffer[i] != ' ' || buffer[i] != '\n')
	{
		if (buffer[i] == ' ' || buffer[i] == '\n')
			break ;
		res.push_back(buffer[i]);
		i++;
	}
	return (res);
}

int Request::acceptRequest(int sockfd)
{
	int valread;
	char	buffer[MAX_REQUEST_SIZE] = {0};

	if ((this->_new_socket = accept(sockfd, (struct sockaddr *)&this->_client_address, &this->_client_addrlen)) < 0)
		throw std::runtime_error("accept failed");

	valread = recv(this->_new_socket, buffer, MAX_REQUEST_SIZE, 0);
	if(valread < 0)
		throw std::runtime_error("No bytes are there to read");
	if (valread == 0)
	{
		std::cout << "Client disconnected" << std::endl;
		return (0);
	}
	else
	{
		std::string raw(buffer, valread);
		parseRequest(raw, this->_new_socket);
		this->_content = whichPage(buffer);
	}
	return (1);
}

std::string const &Request::getContent(void) const{
	return (this->_content);
}

int	Request::getSocket(void) const{
	return (this->_new_socket);
}

void Request::closeSocket(void){
	close(this->_new_socket);
}
