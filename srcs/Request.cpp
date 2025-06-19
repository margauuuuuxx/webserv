#include "../includes/Request.hpp"
#include "../includes/includes.hpp"

Request::Request(void): _clientAddrlen(sizeof(_clientAddress)), _transferEncoding(false), _contentLen(0){}
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

int isRawEmpty(std::string &raw)
{
	size_t count = 0;

	if (raw.empty())
		return (1);
	for (std::string::iterator it = raw.begin(); it != raw.end(); it++)
	{
		if (*it == '\r' || *it == '\n' || *it == ' ')
			count++;
		// if (*it == '\r')
		// 	std::cout << "r" << std::endl;
		// else if (*it == '\n')
		// 	std::cout << "n" << std::endl;
		// else if (*it == ' ')
		// 	std::cout << " " << std::endl;
		// else
		// 	std::cout << (int)*it << std::endl;
	}
	// std::cout << "count" << count << std::endl;
	// std::cout << "raw.length()" <<raw.length() << std::endl;
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
	// std::cout << "méthode: " << method << std::endl;
	// std::cout << "chemin: " << path << std::endl;
	// std::cout << "version: " << version << std::endl;
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

	// std::cout << buffer << std::endl;
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

int Request::acceptRequest(int bytesRead, char buffer[MAX_REQUEST_SIZE], int clientFd)
{
	// int bytesRead;
	// char	buffer[MAX_REQUEST_SIZE] = {0};

	// if ((this->_newSocket = accept(sockfd, (struct sockaddr *)&this->_clientAddress, &this->_clientAddrlen)) < 0)
	// 	throw std::runtime_error("accept failed");
	// std::cout << "Client connecté" << std::endl;

	// bytesRead = recv(this->_newSocket, buffer, MAX_REQUEST_SIZE, 0);
	// if(bytesRead < 0)
	// 	throw std::runtime_error("No bytes are there to read");
	// if (bytesRead == 0)
	// {
	// 	std::cout << "Client disconnected" << std::endl;
	// 	return (0);
	// }
	// else
	// {
		std::string raw(buffer, bytesRead);
		parseRequest(raw, clientFd); //error handling
		this->_content = whichPage(buffer);
	// }
	return (1);
}

std::string const &Request::getContent(void) const{
	return (this->_content);
}

int	Request::getSocket(void) const{
	return (this->_newSocket);
}

int isIncomplete(std::string content)
{
	std::cout << "check dans isChuncked" << std::endl;
	size_t crlf = content.find("\r\n\r\n");
	if (crlf == std::string::npos)
		return (1);
	return (0);
}

void Request::setContentLen(size_t len){
	this->_contentLen = len;
}

void Request::setTransferEncoding(bool state){
	this->_transferEncoding = state;
}

std::string toLower(std::string line, size_t end)
{
	std::string::iterator it = line.begin();
	std::string lower;

	while (it != line.end() && end)
	{
		lower.push_back((char)std::tolower(*it));
		it++;
		end--;
	}
	return (lower);
}

void checkHeaders(Request &obj)
{
	std::istringstream istream(obj.getContent());
	std::string line;
	size_t pos;

	while (std::getline(istream, line))
	{
		std::cout << "line: " << line << std::endl;
		pos = line.find(":");
		if (pos == std::string::npos)
			continue ;
		std::string header = toLower(line, pos);
		if (header == "transfer-encoding")
		{
			obj.setTransferEncoding(true);
			return ;
		}
		else if (header == "content-length")
		{
			std::istringstream value_stream(line);
			size_t value;
			value_stream >> value;
			obj.setContentLen(value);
			return ;
		}
	}
}

int Request::setToParse(char cbuffer[MAX_REQUEST_SIZE]){
	std::string buffer(cbuffer);
	if (isRawEmpty(buffer) && this->_toParse.empty())
		return (0);
	checkHeaders(*this);
	std::cout << "value content-len: " << this->_contentLen << std::endl;
	std::cout << "value transfer-encoding: " << this->_transferEncoding << std::endl;
	std::cout << "buffer: " << buffer << std::endl;
	if (isIncomplete(this->_toParse))
	{
		this->_toParse.append(buffer);
		std::cout << "append: " << this->_toParse << std::endl;
		return (0);
	}
	this->_toParse.assign(buffer);
	std::cout << "assign: " << this->_toParse << std::endl;
	return (1);
}

void Request::closeSocket(void){
	close(this->_newSocket);
}
