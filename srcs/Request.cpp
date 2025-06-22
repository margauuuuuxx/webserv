#include "../includes/Request.hpp"
#include "../includes/includes.hpp"

Request::Request(void): /*_clientAddrlen(sizeof(_clientAddress)),*/ _transferEncoding(false), _waitingForData(false), _contentLen(std::string::npos), _contentLenCopy(std::string::npos){}
Request::~Request(void) {}

// std::string makeError(int code, std::string message)
// {
// 	std::ostringstream oss;

// 	oss << "HTTP/1.1 " << code << " " << message << "\r\n"
// 		<< "Content-Length: 0\r\n"
// 		<< "Connection: close\r\n"
// 		<< "\r\n";
// 	return (oss.str());
// }

std::string makeError(int code, const std::string& message)
{
	std::ostringstream oss;
	std::ostringstream body;

	body << "<html><body><h1>" << code << " " << message << "</h1></body></html>";

	oss << "HTTP/1.1 " << code << " " << message << "\r\n"
		<< "Content-Type: text/html\r\n"
		<< "Content-Length: " << body.str().size() << "\r\n"
		<< "Connection: close\r\n"
		<< "\r\n"
		<< body.str();

	return oss.str();
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

std::string const &Request::getToParse(void) const{
	return (this->_toParse);
}

size_t const &Request::getContentLen(void) const{
	return (this->_contentLen);
}

size_t const &Request::getContentLenCopy(void) const{
	return (this->_contentLenCopy);
}

bool const &Request::getTransferEncoding(void) const{
	return (this->_transferEncoding);
}

bool const &Request::getWaitingState(void) const{
	return (this->_waitingForData);
}

int	Request::getSocket(void) const{
	return (this->_newSocket);
}

void Request::setContentLen(size_t len){
	this->_contentLen = len;
	this->_contentLenCopy = len;
}

void Request::setTransferEncoding(bool state){
	this->_transferEncoding = state;
}

void Request::decrementLen(void){
	if (this->_contentLenCopy - this->_appendLen > 0)
	{
		std::cout << this->_contentLenCopy << " - " << this->_appendLen;
		this->_contentLenCopy -= this->_appendLen;
		std::cout << " = " << this->_contentLenCopy << std::endl;
	}
	else
	{
		std::cout << "contentLenCopy = 0" << std::endl;
		this->_contentLenCopy = 0;
	}
}

int isIncomplete(Request &obj, std::string &content)
{
	// std::cout << "check dans isChuncked" << std::endl;
	size_t crlf = content.find("\r\n\r\n");
	if (crlf == std::string::npos)
		return (1);
	size_t contentLen = obj.getContentLen();
	size_t contentLenCopy = obj.getContentLenCopy();
	if (obj.getTransferEncoding() && contentLenCopy != 0)
		return (2);
	else if (contentLen != 0 && contentLen != std::string::npos && contentLenCopy != 0 && contentLenCopy != std::string::npos)
	{
		// std::cout << "content-length detected" << std::endl;
		// obj.decrementLen();
		return (2);
	}
	// else if (contentLen != 0 && contentLen != std::string::npos && obj.getTransferEncoding())
	// {
	// 	// std::cout << "transfer-encoding detected" << std::endl;
	// 	return (1);
	// }
	return (0);
}

std::string toLower(std::string line, size_t end)
{
	std::string::iterator it = line.begin();
	std::string lower;

	// std::cout << "dans toLower, line: " << line << std::endl;
	while (it != line.end() && end)
	{
		lower.push_back((char)std::tolower(*it));
		it++;
		end--;
	}
	// std::cout << "dans toLower, lower: " << lower << std::endl;
	return (lower);
}

void detectBodyHeader(Request &obj)
{
	std::istringstream istream(obj.getToParse());
	std::string line;
	size_t pos;

	// std::cout << "\e[0;32mavant la boucle de detectBodyHeader\e[0m" << std::endl;
	if (obj.getContentLen() != std::string::npos || obj.getTransferEncoding())
		return ;
	while (std::getline(istream, line, '\r'))
	{
		// std::cout << "line: " << line << std::endl;
		pos = line.find(": ");
		if (pos == std::string::npos)
			continue ;
		std::string header(toLower(line, pos));
		// std::cout << "header: " << header << std::endl;
		if (header.find("transfer-encoding") != std::string::npos)
		{
			obj.setTransferEncoding(true);
			std::cout << "transfer-encoding = true" << std::endl;
			return ;
		}
		else if (header.find("content-length") != std::string::npos)
		{
			std::istringstream value_stream(line);
			size_t value;
			value_stream >> header >> value;
			// std::cout << "new value for content-length: " << value << std::endl;
			obj.setContentLen(value);
			return ;
		}
	}
	// std::cout << "\e[0;32maprès la boucle de detectBodyHeader\e[0m" << std::endl;
}

size_t countLenTransferEncoding(std::string buffer)
{
	std::istringstream iss(buffer);
	size_t len;

	iss >> len;
	std::cout << "len: " << len << std::endl;
	return (len);
}

int Request::setToParse(char cbuffer[MAX_REQUEST_SIZE]){
	std::string buffer(cbuffer);
	int isHeader = 0;

	if (isRawEmpty(buffer) && this->_toParse.empty())
		return (0);
	// std::cout << "\e[0;33msetToParse()\e[0m" << std::endl;
	detectBodyHeader(*this);
	// std::cout << "\e[0;33mafter detectBodyHeader()\e[0m" << std::endl;
	// std::cout << "\e[0;31mvalue content-len: " << this->_contentLen << "\e[0m" << std::endl;
	// std::cout << "\e[0;31mvalue transfer-encoding: " << this->_transferEncoding << "\e[0m" << std::endl;
	// std::cout << "\e[0;31mvalue _contentLenCopy: " << this->_contentLenCopy << "\e[0m" << std::endl;
	// std::cout << "buffer: " << buffer << std::endl;
	if ((isHeader = isIncomplete(*this, this->_toParse)) > 0)
	{
		// this->_toParse.append(buffer);
		if (isHeader != 1 && this->getTransferEncoding())
		{
			std::cout << "isIncomplete in getTransferEncoding condition" << std::endl;
			this->_toParse.append(buffer);
			if (!this->_waitingForData)
				this->_contentLenCopy = countLenTransferEncoding(buffer);
			this->_waitingForData = !this->_waitingForData;
			std::cout << "TransferEncoding: " << this->_contentLen << std::endl;
			if (this->_contentLenCopy != 0 && isRawEmpty(buffer))
			{
				this->_toParse.assign(makeError(400, "Bad request"));
				return (1);
			}
			if (this->_contentLenCopy == 0)
				return (1);
		}
		else if (isHeader != 1 && this->getContentLenCopy() != 0 && this->getContentLenCopy() != std::string::npos)
		{
			// this->_toParse.append(buffer);
			this->_appendLen = buffer.length() - 2;
			if ((long int)this->_contentLenCopy - (long int)this->_appendLen >= 0)
			{
				this->_contentLenCopy -= this->_appendLen;
				this->_toParse.append(buffer);
			}
			else
			{
				this->_toParse.append(buffer, 0, this->_contentLenCopy);
				return (1);
			}
			if (this->_contentLenCopy == 0)
				return (1);
			std::cout << "content-length: " << this->_appendLen << std::endl;
		}
		else
			this->_toParse.append(buffer);
		std::cout << "buffer length: " << buffer.length() - 2 << std::endl;
		std::cout << this->_toParse << std::endl << std::endl;
		return (0);
	}
	else
	{
		this->_toParse.assign(buffer);
		std::cout << "assign: " << this->_toParse << std::endl;
	}
	std::cout << "retourne 1" << std::endl;
	return (1);
}

void Request::closeSocket(void){
	close(this->_newSocket);
}
