#include "../includes/Request.hpp"
#include "../includes/includes.hpp"

Request::Request(void): /*_clientAddrlen(sizeof(_clientAddress)),*/ _transferEncoding(false), _waitingForData(false), _contentLen(std::string::npos), _contentLenCopy(std::string::npos){}
Request::~Request(void) {}

int isRawEmpty(std::string &raw)
{
	size_t count = 0;

	if (raw.empty())
		return (1);
	for (std::string::iterator it = raw.begin(); it != raw.end(); it++)
	{
		if (*it == '\r' || *it == '\n' || *it == ' ')
			count++;
	}
	if (count == raw.length())
		return (1);
	return (0);
}

// void parseRequest(std::string raw, int sockfd)
// {
// 	std::istringstream iss(raw);
// 	std::string method;
// 	std::string path;
// 	std::string version;
// 	std::ifstream test;

// 	if (isRawEmpty(raw))
// 		return ;
// 	iss >> method >> path >> version;
// 	// std::cout << "méthode: " << method << std::endl;
// 	// std::cout << "chemin: " << path << std::endl;
// 	// std::cout << "version: " << version << std::endl;
// 	if (method != "GET" && method != "POST" && method != "DELETE")
// 	{
// 		std::string error = makeError(405, "Method Not Allowed");
// 		send(sockfd, error.c_str(), error.length(), 0);
// 		// throw std::runtime_error("mauvaise méthode");
// 		close(sockfd);
// 		return ;
// 	}
// 	if (version != "HTTP/1.1")
// 	{
// 		std::cerr << "probleme version" << std::endl;
// 		throw std::runtime_error("mauvaise version");
// 	}
// 	std::string file = path.substr(1);
// 	if (!file.empty())
// 	{
// 		test.open(file.c_str());
// 		if (!test.is_open())
// 		{
// 			std::cerr << "probleme page" << std::endl;
// 			throw std::runtime_error("mauvaise page");
// 		}
// 		test.close();
// 	}
// }

// std::string whichPage(char buffer[MAX_REQUEST_SIZE])
// {
// 	int i = 0;
// 	std::string res;

// 	// std::cout << buffer << std::endl;
// 	while (buffer[i])
// 	{
// 		if (buffer[i] == '/' && buffer[i + 1])
// 		{
// 			i++;
// 			break ;
// 		}
// 		i++;
// 	}
// 	while (buffer[i] != ' ' || buffer[i] != '\n')
// 	{
// 		if (buffer[i] == ' ' || buffer[i] == '\n')
// 			break ;
// 		res.push_back(buffer[i]);
// 		i++;
// 	}
// 	return (res);
// }

// int Request::acceptRequest(int bytesRead, char buffer[MAX_REQUEST_SIZE], int clientFd)
// {
// 	// int bytesRead;
// 	// char	buffer[MAX_REQUEST_SIZE] = {0};

// 	// if ((this->_newSocket = accept(sockfd, (struct sockaddr *)&this->_clientAddress, &this->_clientAddrlen)) < 0)
// 	// 	throw std::runtime_error("accept failed");
// 	// std::cout << "Client connecté" << std::endl;

// 	// bytesRead = recv(this->_newSocket, buffer, MAX_REQUEST_SIZE, 0);
// 	// if(bytesRead < 0)
// 	// 	throw std::runtime_error("No bytes are there to read");
// 	// if (bytesRead == 0)
// 	// {
// 	// 	std::cout << "Client disconnected" << std::endl;
// 	// 	return (0);
// 	// }
// 	// else
// 	// {
// 		std::string raw(buffer, bytesRead);
// 		parseRequest(raw, clientFd); //error handling
// 		this->_content = whichPage(buffer);
// 	// }
// 	return (1);
// }

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

void Request::setContentLen(size_t len){
	this->_contentLen = len;
	this->_contentLenCopy = len;
}

void Request::setTransferEncoding(bool state){
	this->_transferEncoding = state;
}

void Request::reset(void)
{
	this->_appendLen = 0;
	this->_content.clear();
	this->_contentLen = std::string::npos;
	this->_contentLenCopy = std::string::npos;
	this->_toParse.clear();
	this->_transferEncoding = false;
	this->_waitingForData = false;
}

int isIncomplete(Request &obj, std::string &content)
{
	size_t crlf = content.find("\r\n\r\n");
	if (crlf == std::string::npos)
		return (1);
	size_t contentLen = obj.getContentLen();
	size_t contentLenCopy = obj.getContentLenCopy();
	if (obj.getTransferEncoding() && contentLenCopy != 0)
		return (2);
	else if (contentLen != 0 && contentLen != std::string::npos && contentLenCopy != 0 && contentLenCopy != std::string::npos)
		return (2);
	return (0);
}

void detectBodyHeader(Request &obj)
{
	std::istringstream istream(obj.getToParse());
	std::string line;
	size_t pos;

	if (obj.getContentLen() != std::string::npos || obj.getTransferEncoding())
		return ;
	while (std::getline(istream, line, '\r'))
	{
		pos = line.find(": ");
		if (pos == std::string::npos)
			continue ;
		std::string header(toLower(line, pos));
		std::istringstream value_stream(line);
		if (header.find("transfer-encoding") != std::string::npos)
		{
			std::cout << "transfer-encoding trouvé" << std::endl;
			std::string chunked;
			value_stream >> header >> chunked;
			if (chunked.find("chunked") == std::string::npos)
				obj.setContentLen(std::string::npos - 1);
			else
				obj.setTransferEncoding(true);
			return ;
		}
		else if (header.find("content-length") != std::string::npos)
		{
			size_t value;
			if (!(value_stream >> header >> value))
				obj.setContentLen(std::string::npos - 1);
			else
				obj.setContentLen(value);
			return ;
		}
	}
}

size_t countLenTransferEncoding(std::string buffer)
{
	std::istringstream iss(buffer);
	size_t len;

	if (!(iss >> std::hex >> len))
		return (std::string::npos);
	std::cout << "len: " << std::hex << len << std::endl;
	return (len);
}

int Request::setToParse(char cbuffer[MAX_REQUEST_SIZE]){
	std::string buffer(cbuffer);
	int isHeader = 0;

	if (isRawEmpty(buffer) && this->_toParse.empty())
		return (0);
	detectBodyHeader(*this);
	if (this->_contentLen == std::string::npos - 1)
	{
		this->_toParse.assign(makeError(400, "Bad Request"));
		return (1);
	}
	if ((isHeader = isIncomplete(*this, this->_toParse)) > 0)
	{
		if (isHeader != 1 && this->getTransferEncoding())
		{
			std::cout << "isIncomplete in getTransferEncoding condition" << std::endl;
			if (!this->_waitingForData)
			{
				this->_contentLenCopy = countLenTransferEncoding(buffer);
				if (this->_contentLenCopy == std::string::npos)
				{
					this->_toParse.assign(makeError(400, "Bad Request"));
					return (1);
				}
			}
			this->_waitingForData = !this->_waitingForData;
			std::cout << "TransferEncoding: " << this->_contentLen << std::endl;
			if ((!this->_waitingForData && buffer.size() - 2 < this->_contentLenCopy) || (this->_contentLenCopy != 0 && isRawEmpty(buffer)))
			{
				std::cout << "buffer size: " << buffer.size() - 2 << " contentLenCopy: " << this->_contentLenCopy << std::endl;
				this->_toParse.assign(makeError(400, "Bad Request"));
				return (1);
			}
			if (!this->_waitingForData)
				for (size_t i = 0; i != this->_contentLenCopy + 2; i++)
					this->_toParse.push_back(buffer.at(i));
			else
				this->_toParse.append(buffer);
			if (this->_contentLenCopy == 0)
				return (1);
		}
		else if (isHeader != 1 && this->getContentLenCopy() != 0 && this->getContentLenCopy() != std::string::npos)
		{
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
	}
	return (1);
}
