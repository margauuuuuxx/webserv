#include "../includes/includes.hpp"

/*
	This class is responsible for building the appropriate HTTP response based on the parsed Request object and the Server config.
*/

Request::Request(void): /*_clientAddrlen(sizeof(_clientAddress)),*/ _error(false), _transferEncoding(false), _waitingForData(false), _contentLen(std::string::npos), _contentLenCopy(std::string::npos){}
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

std::string const &Request::getContent(void) const{
	return (this->_content);
}

std::string	const	&Request::getVersion(void) const {
	return (this->_version);
}

std::map<std::string, std::string>	const	&Request::getHeaders(void) const {
	return (this->_headers);
}

std::string	const	&Request::getBody(void) const {
	return (this->_body);
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

bool const &Request::getErrorFlag(void) const {
	return (this->_error);
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
	{
		std::cout << "pas de CRLFCRLF" << std::endl;
		return (1);
	}
	size_t contentLen = obj.getContentLen();
	size_t contentLenCopy = obj.getContentLenCopy();
	if (obj.getTransferEncoding() && contentLenCopy != 0)
	{
		std::cout << "transfer encoding pas termine" << std::endl;
		return (2);
	}
	else if (contentLen != 0 && contentLen != std::string::npos && contentLenCopy != 0 && contentLenCopy != std::string::npos)
	{
		std::cout << "content-length pas termine" << std::endl;
		return (2);
	}
	return (0);
}

/*
	La fonction detectBodyHeader sert à vérifier si un content-length
	ou un transfer-encoding est présent.
	Les deux headers ne sont pas compatibles et donc ne peuvent pas être ensemble.
	La fonction prévient les doublons et la compatibilité.
	Elle met aussi à jour les variables associées aux headers.
*/
int detectBodyHeader(Request &obj)
{
	std::istringstream istream(obj.getToParse());
	std::string line;
	size_t pos;
	int res = 0;

	// std::cout << "to parse dans body header: " << obj.getToParse() << std::endl;
	// std::cout << "detect body header" << std::endl;
	while (std::getline(istream, line, '\n'))
	{
		if (res >= 2)
		{
			std::cout << "res = " << res << " au début de la boucle" << std::endl;
			return (res);
		}
		pos = line.find(":");
		if (pos == std::string::npos)
			continue ;
		std::cout << "line: " << line << std::endl;
		std::string header(toLower(line, pos + 1));
		std::cout << "header: " << header << std::endl;
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
			std::cout << "res: " << res << std::endl;
		}
		else if (header.find("content-length:") != std::string::npos)
		{
			size_t value;
			if (!(value_stream >> header >> value))
				obj.setContentLen(std::string::npos - 1);
			else
				obj.setContentLen(value);
			res++;
			std::cout << "res: " << res << std::endl;
		}
	}
	// std::cout << "tout est normal, on retourne " << res << std::endl; 
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

/*
	setToParse fais un petit parsing mais sert surtout à donner
	une requête complète a la vraie fonction de parsing.
*/
int Request::setToParse(char cbuffer[MAX_REQUEST_SIZE]){
	std::string buffer(cbuffer);
	int isHeader = 0;

	if (this->_toParse.empty() && isRawEmpty(buffer))
		return (0);
	// std::cout << "toParse: " << this->_toParse << std::endl;
	if (detectBodyHeader(*this) >= 2 || this->_contentLen == std::string::npos - 1)
		return (assignError(makeError(400, "Bad Request for header")));
	std::cout << "detectBodyHeader est passé" << std::endl;
	if ((this->_contentLen == std::string::npos && !this->getTransferEncoding()) && buffer.size() == 2 && ((int)buffer.at(0) == 13) && ((int)buffer.at(1) == 10))
	{
		this->_toParse.append(buffer);
		std::cout << "on retourne 1" << std::endl;
		return (1);
	}
	if (!checkCRLF(buffer))
	{
		std::cout << "problème de CRLF" << std::endl;
		return (assignError(makeError(400, "Bad Request for CRLF")));
	}
	std::cout << "On check si complet" << std::endl;
	/*
		La première fois que cette fonction est appelé, _toParse est vide et on part
		du principe que le buffer contient une requête complète.
		Donc on l'assigne tout simplement.
	*/
	if (this->_toParse.empty())
		this->_toParse.assign(buffer);
	/*
		On vérifie que ce soit le cas avec la fonction isIncomplete(). Si elle est
		incomplète, on efface _toParse pour recommencer et faire les choses BIEN.
	*/
	if ((isHeader = isIncomplete(*this, this->_toParse)) > 0)
	{
		if (this->_toParse == buffer)
			this->_toParse.clear();
		std::cout << "Requête pas complète" << std::endl;
		if (isHeader != 1 && this->getTransferEncoding())
		{
			/*
				Un body sous transfer-encoding: chunked ressemble à ceci:
				3
				ccc
				a
				cccccccccc
				Il faut donc, avoir le nombre de caractères donnés, puis
				recevoir les caractères.
				Si la fonction recoit un nombre et que les données reçues
				ont trop de caractères, il coupe ce qu'il "dépasse".
				Si par contre la les données reçues ne sont pas assez nombreuses
				alors la fonction renvoit une erreur.
			*/
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
			{
				std::cout << "on retourne 1" << std::endl;
				return (1);
			}
		}
		else if (isHeader != 1 && this->getContentLenCopy() != 0 && this->getContentLenCopy() != std::string::npos)
		{
			/*
				Le nombre de caractères à recevoir est dans les headers.
				Si il y a trop de caractères, on coupe le trop plein.
				Si il n'y en a pas assez, alors on attends 10 secondes avant
				de renvoyer un timeout.
			*/
			this->_appendLen = buffer.length() - 2;
			if ((long int)this->_contentLenCopy - (long int)this->_appendLen >= 0)
			{
				this->_contentLenCopy -= this->_appendLen;
				this->_toParse.append(buffer);
			}
			else
			{
				this->_toParse.append(buffer, 0, this->_contentLenCopy);
				std::cout << "on retourne 1" << std::endl;
				return (1);
			}
			if (this->_contentLenCopy == 0)
			{
				std::cout << "on retourne 1" << std::endl;
				return (1);
			}
		}
		else
			this->_toParse.append(buffer);
		return (0);
	}
	std::cout << "on retourne 1" << std::endl;
	return (1);
}

void Request::parse(void){
	std::istringstream iss(this->_toParse);
	std::string line;
	std::string currentKey;
	bool isBody = false;
	int i = 0;

	if (this->_error)
		return ;
	std::cout << "-----------PARSE-----------" << std::endl;
	while (std::getline(iss, line, '\n'))
	{
		/*
			Recheck et suppression de CRLF
		*/
		if (line.at(line.size() - 1) != 13)
			return ((void)assignError(makeError(400, "Bad Request CRLF in parse()")));
		line.erase(line.size() - 1);
		std::istringstream issLine(line);
		if (i == 0)
		{
			/*
				Parsing de la request line (methode, fichier, version)
			*/
			if (line.size() > MAX_REQUEST_LINE_SIZE)
				return ((void)assignError(makeError(413, "Content Too Large")));
			issLine >> this->_method >> this->_content >> this->_version;
			issLine >> std::ws;
			if (this->_method.empty() || this->_content.empty() || this->_version.empty()
				|| !issLine.eof()
				|| this->_version != "HTTP/1.1")
				return ((void)assignError(makeError(400, "Bad Request request line")));
			else if (this->_method != "GET" && this->_method != "POST" && this->_method != "DELETE")
				return ((void)assignError(makeError(405, "Method Not Allowed")));
		}
		else if (!line.empty() && !isBody)
		{
			/*
				Parsing des headers
			*/
			if (line.size() > MAX_REQUEST_LINE_SIZE)
				return ((void)assignError(makeError(413, "Content Too Large")));
			if (std::isspace(line.at(0)))
				this->_headers[currentKey].append(line);
			else
			{
				size_t colon = line.find(':');
				if (colon == std::string::npos)
					return ((void)assignError(makeError(400, "Bad Request pas de ':'")));
				std::string key = toLower(line, colon);
				currentKey = key;
				std::string value = ftStrtrim(line.substr(colon + 1));
				std::map<std::string, std::string>::iterator it;
				if (key == "authorization" || key == "proxy-authorization")
					this->_multiHeaders.insert(std::pair<std::string, std::string>(key, value));
				else if ((it = this->_headers.find(key)) == this->_headers.end())
					this->_headers[key] = value;
				else if (!uniqueHeaders.count(key))
						it->second.append(", " + value);
				else
					return ((void)assignError(makeError(400, "Bad Request multiple unique header")));
			}
		}
		else
		{
			if (!isBody)
				isBody = true;
			this->_body.append(line);
		}
		i++;
	}
	if (!this->_headers.count("host"))
		return ((void)assignError(makeError(400, "Bad Request pas de host")));
	if (this->_headers.size() > MAX_HEADERS_SIZE)
		return ((void)assignError(makeError(413, "Content Too Large")));
	std::cout << "method:	" << this->_method << std::endl;
	std::cout << "content:	" << this->_content << std::endl;
	std::cout << "version:	" << this->_version << std::endl;
	std::cout << "map:" << std::endl;
	for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
	{
		std::cout << "first:	" << it->first << ", ";
		std::cout << "second:	" << it->second << std::endl;
	}
	std::cout << "multimap:" << std::endl;
	for (std::multimap<std::string, std::string>::iterator it = this->_multiHeaders.begin(); it != this->_multiHeaders.end(); it++)
	{
		std::cout << "first:	" << it->first << ", ";
		std::cout << "second:	" << it->second << std::endl;
	}
	std::cout << "Body: " << this->_body << std::endl;
}
