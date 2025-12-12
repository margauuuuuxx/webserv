#pragma once

#include "includes.hpp"

class Request
{
public:
	Request(void);
	~Request(void);

	std::string	const							&getMethod(void) const;
	std::string	const							&getContent(void) const;
	std::string	const							&getVersion(void) const;
	std::string const							&getClientIP(void) const;
	std::map<std::string, std::string>	const	&getHeaders(void) const;
	const std::vector<char>						&getBody(void) const;
	size_t		const							&getContentLen(void) const;
	void										appendBody(char *cbuffer, size_t bytes);
	int											getErrorCode(void) const;
	const std::string							&getStatusMessage(void) const;
	void										assignError(int code, const std::string& message);
	void										appendToRawRequest(const char* buffer, int bytes);
	void										setClientIP(const std::string& ip);
	void										parse(size_t clientMaxBodySize);
	void										reset(void);
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
	size_t									_contentLen;
	std::string								_method;
	std::string								_content;
	std::string								_version;
	std::map <std::string, std::string>		_headers;
	std::multimap<std::string, std::string>	_multiHeaders;
	std::vector<char>		 				_body;
	std::string								_clientIP;
	ParsingState							_parsingState;
	int										_errorCode;
	std::string								_statusMessage;
};
