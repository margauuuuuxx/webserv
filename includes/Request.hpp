#pragma once

#include "includes.hpp"

class Request
{
public:
	Request(void);
	~Request(void);

	int											acceptRequest(int bytesRead, char buffer[MAX_REQUEST_SIZE], int clientFd);
	std::string	const							&getMethod(void) const;
	std::string	const							&getContent(void) const;
	std::string	const							&getVersion(void) const;
	std::string const							&getClientIP(void) const;
	std::map<std::string, std::string>	const	&getHeaders(void) const;
	const std::vector<char>						&getBody(void) const;
	std::string	const							&getRawRequest(void) const;
	size_t		const							&getContentLen(void) const;
	size_t		const							&getContentLenCopy(void) const;
	void										appendBody(char *cbuffer, size_t bytes);
	bool		const							&getTransferEncoding(void) const;
	bool		const							&getWaitingState(void) const;
	int											getErrorCode(void) const;
	const std::string							&getStatusMessage(void) const;
	int											getSocket(void) const;
	bool		const							&getErrorFlag(void) const;
	void										assignError(int code, const std::string& message);
	void										appendToRawRequest(const char* buffer, int bytes);
	//int											setToParse(char buffer[MAX_REQUEST_SIZE], int const bytes);
	void										setContentLen(size_t len);
	void										setTransferEncoding(bool state);
	void										setClientIP(const std::string& ip);
	void										parse(void);
	void										reset(void);
	bool										requestState(void);
	bool										parsingFinished() const;
	bool										parsingError() const;

	enum ParsingState {
		PARSING_REQUEST_LINE,
		PARSING_HEADERS,
		PARSING_BODY,
		PARSING_CHUNKED_BODY,
		PARSING_DONE,
		PARSING_ERROR
	};

private:
	std::string								_rawRequest;
	bool									_error;
	bool									_transferEncoding;
	bool									_waitingForData;
	size_t									_contentLen;
	size_t									_contentLenCopy;
	size_t									_appendLen;
	std::string								_method;
	std::string								_content;
	std::string								_version;
	std::map <std::string, std::string>		_headers;
	std::multimap<std::string, std::string>	_multiHeaders;
	std::vector<char>		 				_body;
	std::string								_clientIP;
	bool									_requestFinished;
	ParsingState							_parsingState;
	int										_errorCode;
	std::string								_statusMessage;
};
