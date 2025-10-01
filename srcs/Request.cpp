#include "../includes/includes.hpp"

/*
	This class is responsible for building the appropriate HTTP response based on the parsed Request object and the Server config.
*/

Request::Request(void) {this->reset();}

Request::~Request(void) {}

const std::string headersArray[] = {
	"host",
	"content-length",
	"content-type",
	"date",
	"user-agent",
	"authorization",
	"referer",
	"range",
	"max-forwards",
	"cookie",
	"END_OF_ARRAY"
};

std::set<std::string> createSet(void) {
	std::set<std::string> set;
	size_t max = 0;

	while (headersArray[max] != "END_OF_ARRAY")
		max++;
	for (size_t i = 0; i < max; i++)
		set.insert(headersArray[i]);
	return (set);
}

const std::set<std::string> uniqueHeaders = createSet();

/*
	La fonction vérifie si la string contient uniquement des whitespaces.
*/
int isRawEmpty(std::string &raw)
{
	size_t count = 0;

	if (raw.empty())
		return (1);
	for (std::string::iterator it = raw.begin(); it != raw.end(); it++)
	{
		if (std::isspace(*it))
			count++;
	}
	if (count == raw.length())
		return (1);
	return (0);
}

std::string	const	&Request::getMethod(void) const {
	return (this->_method);
}

std::string const	&Request::getClientIP(void) const {
	return (this->_clientIP);
}
 
std::string const &Request::getContent(void) const{
	return (this->_content);
}

std::string	const	&Request::getVersion(void) const {
	return (this->_version);
}

std::map<std::string, std::string>	const	&Request::getHeaders(void) const {
	return (this->_headers);
}

const std::vector<char>	&Request::getBody(void) const { return (this->_body); }

std::string const &Request::getRawRequest(void) const {
	return (this->_rawRequest);
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

bool const &Request::getErrorFlag(void) const {
	return (this->_error);
}

int	Request::getErrorCode(void) const { return (this->_errorCode); }

const std::string	&Request::getStatusMessage(void) const { return (this->_statusMessage); }

void Request::setContentLen(size_t len){
	this->_contentLen = len;
	this->_contentLenCopy = len - 1;
}

void Request::setTransferEncoding(bool state){
	this->_transferEncoding = state;
}

void	Request::setClientIP(const std::string& ip) {
	this->_clientIP = ip;
}

void	Request::assignError(int code, const std::string& message) {
	this->_error = true;
	this->_parsingState = PARSING_ERROR;
	this->_errorCode = code;
	this->_statusMessage = message;
}

void Request::reset(void)
{
	this->_appendLen = 0;
	this->_contentLen = std::string::npos;
	this->_contentLenCopy = std::string::npos;
	this->_transferEncoding = false;
	this->_waitingForData = false;
	this->_error = false;
	this->_content.clear();
	this->_rawRequest.clear();
	this->_method.clear();
	this->_version.clear();
	this->_headers.clear();
	this->_multiHeaders.clear();
	this->_parsingState = PARSING_REQUEST_LINE;
	this->_errorCode = 0; 
	this->_statusMessage.clear();
}

size_t countLenTransferEncoding(std::string buffer)
{
	std::istringstream iss(buffer);
	size_t len;

	if (!(iss >> std::hex >> len))
		return (std::string::npos);
	
	return (len);
}

int checkCRLF(std::string &buffer)
{
	for (size_t i = 1; i < buffer.size(); ++i)
	{
		if (buffer[i] == '\r' && i + 1 >= buffer.size() && buffer[i + 1] != '\n')
			return (0);
		if (buffer[i] == '\n' && buffer[i - 1] != '\r')
			return (0);
	}
	return (1);
}

void	Request::appendBody(char *cbuffer, size_t bytes) {
	if (cbuffer && bytes > 0)
		this->_body.insert(this->_body.end(), cbuffer, cbuffer + bytes);
}

void	Request::appendToRawRequest(const char* buffer, int bytes) {
	this->_rawRequest.append(buffer, bytes);
}

void Request::parse(void){
	while (_parsingState != PARSING_DONE && _parsingState != PARSING_ERROR) {
		switch(_parsingState) {
			case PARSING_REQUEST_LINE: {
				size_t pos = _rawRequest.find("\r\n");
				if (pos != std::string::npos) {
					std::string line = _rawRequest.substr(0, pos);
					
					if (line.size() > MAX_REQUEST_LINE_SIZE) {
						assignError(413, "Content Too Large"); // CHECK THE USE OF THESE FCTS
						DEBUG_LOG(RED << "Error: " << RESET << "413 Content Too Large in the parsing of the request line");
						return;
					}

					std::istringstream iss(line);
					iss >> this->_method >> this->_content >> this->_version;
					iss >> std::ws; // for trailing whitespace

					if (this->_method.empty() || this->_content.empty() || this->_version.empty()
						|| !iss.eof()
						|| this->_version != "HTTP/1.1") {
							assignError(400, "Bad Request");
							DEBUG_LOG(RED << "Error: " << RESET << "400 Bad Request in request line parsing");
							return;
						}
					else if (this->_method != "GET" && this->_method != "POST" && this->_method != "DELETE") {
						assignError(405, "Method Not Allowed");
						DEBUG_LOG(RED << "Error: " << RESET << "405 Method Not Allowed in request line parsing");
						return;
					}

					_parsingState = PARSING_HEADERS;
					_rawRequest.erase(0, pos + 2);
				} else 
					return;
				break;
			}
			case PARSING_HEADERS: {
				size_t pos = _rawRequest.find("\r\n\r\n");
				if (pos != std::string::npos) {
					std::string headers_part = _rawRequest.substr(0, pos);
					std::istringstream iss(headers_part);
					std::string line;

					while (std::getline(iss, line)) {
						if (line.empty() || line[0] == '\r')
							continue;
						if (line[line.size() - 1] == '\r')
							line.erase(line.size() - 1);
						
						size_t colon_pos = line.find(':');
						if (colon_pos == std::string::npos) {
							assignError(400, "Bad Request");
							DEBUG_LOG(RED << "Error: " << RESET << "400 Bad Request in headers parsing");
							return;
						}

						std::string key = toLower(line, colon_pos);
						std::string value = line.substr(colon_pos + 1);
						value.erase(0, value.find_first_not_of(" \t")); // trim whitespace

						if (this->_headers.count(key) && uniqueHeaders.count(key)) {
							assignError(400, "Bad Request");
							DEBUG_LOG(RED << "Error: " << RESET << "400 Bad Request in headers parsing");
							return;
						}
						this->_headers[key] += value;
					}

					if (!this->_headers.count("host")) {
						assignError(400, "Bad Request");
						DEBUG_LOG(RED << "Error: " << RESET << "400 Bad Request in headers parsing");
						return;
					}
					if (this->_headers.size() > MAX_HEADERS_SIZE) {
						assignError(413, "Content Too Large");
						DEBUG_LOG(RED << "Error: " << RESET << "413 Content Too Large in headers parsing");
						return;
					}

					if (this->_headers.count("transfer-encoding") 
						&& this->_headers["transfer-encoding"].find("chunked") != std::string::npos) {
							if (this->_headers.count("content-length")) {
								assignError(400, "Bad Request");
								DEBUG_LOG(RED << "Error: " << RESET << "400 Bad Request in headers parsing");
								return;
							}
							this->_transferEncoding = true;
							this->_parsingState = PARSING_CHUNKED_BODY;
						}
					else if (this->_headers.count("content-length")) {
						std::istringstream iss(this->_headers["content-length"]);
						size_t contentLenVal;

						if (!(iss >> contentLenVal) || iss.rdbuf()->in_avail() != 0) {
							assignError(400, "Bad Request");
							DEBUG_LOG(RED << "Error: " << RESET << "400 Bad Request in headers parsing");
							return;
						}

						this->_contentLen = contentLenVal;
						this->_parsingState = PARSING_BODY;
					}
					else 
						this->_parsingState = PARSING_DONE;
					_rawRequest.erase(0, pos + 4);
				} else
					return;
				break;
			}
			case PARSING_BODY: {
				if (this->_contentLen > MAX_BODY_SIZE) {
					assignError(413, "Content Too Large");
					DEBUG_LOG(RED << "Error: " << RESET << "413 Content Too Large in body parsing");
					return;
				}

				if (_rawRequest.length() >= this->_contentLen) {
					_body.assign(_rawRequest.begin(), _rawRequest.begin() + this->_contentLen);
					_rawRequest.erase(0, this->_contentLen);
					_parsingState = PARSING_DONE;
				} else 
					return;
				break;
			}
			case PARSING_CHUNKED_BODY: {
				while (true) {
					size_t line_end = _rawRequest.find("\r\n");
					if (line_end == std::string::npos)
						return;

					std::string size_line = _rawRequest.substr(0, line_end);
					size_t chunk_size;
					std::istringstream iss(size_line);
					if (!(iss >> std::hex >> chunk_size)) {
						assignError(400, "Bad Request");
						DEBUG_LOG(RED << "Error: " << RESET << "400 Bad Request in chunked body parsing");
						return;
					}

					size_t total_chunk_len = line_end + 2 + chunk_size + 2;
					if (_rawRequest.length() < total_chunk_len)
						return;
					
					if (chunk_size == 0) {
						this->_parsingState = PARSING_DONE;
						_rawRequest.erase(0, line_end + 2);
						break;
					}

					appendBody(&_rawRequest[line_end + 2], chunk_size);
					_rawRequest.erase(0, total_chunk_len);
				}
				break;
			}
			case PARSING_DONE:
			case PARSING_ERROR:
				break;
		}
	}
}

bool	Request::parsingFinished() const { return (_parsingState == PARSING_DONE); }

bool	Request::parsingError() const { return (_parsingState == PARSING_ERROR); }
