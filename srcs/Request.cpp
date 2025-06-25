#include "../includes/Request.hpp"
#include "../includes/includes.hpp"

Request::Request(void): /*_clientAddrlen(sizeof(_clientAddress)),*/ _error(false), _transferEncoding(false), _waitingForData(false), _contentLen(std::string::npos), _contentLenCopy(std::string::npos){}
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

int Request::assignError(std::string error){
	this->_toParse.assign(error);
	this->_error = true;
	return (1);
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

int detectBodyHeader(Request &obj)
{
	std::istringstream istream(obj.getToParse());
	std::string line;
	size_t pos;
	int res = 0;

	if (res >= 2)
		return (res);
	while (std::getline(istream, line, '\r'))
	{
		if (res >= 2)
			return (res);
		pos = line.find(": ");
		if (pos == std::string::npos)
			continue ;
		std::string header(toLower(line, pos));
		std::istringstream value_stream(line);
		if (header.find("transfer-encoding:") != std::string::npos)
		{
			std::string chunked;
			value_stream >> header >> chunked;
			if (chunked.find("chunked") == std::string::npos)
				obj.setContentLen(std::string::npos - 1);
			else
				obj.setTransferEncoding(true);
			res++;
		}
		else if (header.find("content-length:") != std::string::npos)
		{
			size_t value;
			if (!(value_stream >> header >> value))
				obj.setContentLen(std::string::npos - 1);
			else
				obj.setContentLen(value);
			res++;
		}
	}
	return (res);
}

size_t countLenTransferEncoding(std::string buffer)
{
	std::istringstream iss(buffer);
	size_t len;

	if (!(iss >> std::hex >> len))
	{
		std::cout << "iss a foiré" << std::endl;
		return (std::string::npos);
	}
	return (len);
}

int checkCRLF(std::string buffer)
{
	size_t bufferLen = buffer.size();
	size_t LF = bufferLen - 1;
	size_t CR = bufferLen - 2;
	if ((int)buffer.at(CR) == 13 && (int)buffer.at(LF) == 10)
		return (1);
	return (0);
}

int Request::setToParse(char cbuffer[MAX_REQUEST_SIZE]){
	std::string buffer(cbuffer);
	int isHeader = 0;

	if (this->_toParse.empty() && isRawEmpty(buffer))
		return (0);
	if (detectBodyHeader(*this) >= 2 || this->_contentLen == std::string::npos - 1)
		return (assignError(makeError(400, "Bad Request for header")));
	if ((this->_contentLen == std::string::npos && !this->getTransferEncoding()) && buffer.size() == 2 && ((int)buffer.at(0) == 13) && ((int)buffer.at(1) == 10))
	{
		this->_toParse.append(buffer);
		return (1);
	}
	if (!checkCRLF(buffer))
		return (assignError(makeError(400, "Bad Request for CRLF")));
	if ((isHeader = isIncomplete(*this, this->_toParse)) > 0)
	{
		if (isHeader != 1 && this->getTransferEncoding())
		{
			if (!this->_waitingForData)
			{
				this->_contentLenCopy = countLenTransferEncoding(buffer);
				if (this->_contentLenCopy == std::string::npos)
					return (assignError(makeError(400, "Bad Request for hexa")));
			}
			this->_waitingForData = !this->_waitingForData;
			if ((!this->_waitingForData && buffer.size() - 2 < this->_contentLenCopy) || (this->_contentLenCopy != 0 && isRawEmpty(buffer)))
				return (assignError(makeError(400, "Bad Request for buffer size")));
			if (!this->_waitingForData)
			{
				for (size_t i = 0; i != this->_contentLenCopy; i++)
					this->_toParse.push_back(buffer.at(i));
				this->_toParse.push_back('\r');
				this->_toParse.push_back('\n');
			}
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
		}
		else
			this->_toParse.append(buffer);
		return (0);
	}
	else
		this->_toParse.assign(buffer);
	return (1);
}

void Request::parse(void){
	std::istringstream iss(this->_toParse);
	std::string line;
	std::string currentToken;
	int i = 0;

	if (this->_error)
		return ;
	while (std::getline(iss, line, '\n'))
	{
		line.erase(line.size() - 1);
		std::istringstream issLine(line);
		if (i == 0)
		{
			issLine >> this->_method >> this->_content >> this->_version;
			issLine >> std::ws;
			if (this->_method.empty() || this->_content.empty() || this->_version.empty()
				|| !issLine.eof()
				|| this->_version != "HTTP/1.1")
				return ((void)assignError(makeError(400, "Bad Request")));
			else if (this->_method != "GET" && this->_method != "POST" && this->_method != "DELETE")
				return ((void)assignError(makeError(405, "Method Not Allowed")));
		}
		else if (!line.empty())
		{
			if (std::isspace(line.at(0)))
				this->_headers[currentToken].append(line);
			else
			{
				size_t colon = line.find(':');
				if (colon == std::string::npos)
					return ((void)assignError(makeError(400, "Bad Request")));
				std::string token = toLower(line, colon);
				currentToken = token;
				std::string value = line.substr(colon + 1);
				if (!this->_headers[token].empty())
					std::cout << "doublon?" << std::endl;
				this->_headers[token] = value;
			}
		}
		i++;
	}
	std::cout << "method	:" << this->_method << std::endl;
	std::cout << "content	:" << this->_content << std::endl;
	std::cout << "version	:" << this->_version << std::endl;
	for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
	{
		std::cout << "first	:" << it->first << std::endl;
		std::cout << "second	:" << it->second << std::endl;
	}
}
