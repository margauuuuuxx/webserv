#include "../includes/includes.hpp"

Request::Request(void): /*_clientAddrlen(sizeof(_clientAddress)),*/ _error(false), _transferEncoding(false), _waitingForData(false), _contentLen(std::string::npos), _contentLenCopy(std::string::npos), _requestFinished(false){}
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

void	Request::setClientIP(const std::string& ip) {
	this->_clientIP = ip;
}

int Request::assignError(std::string error){
	this->_toParse.assign(error);
	this->_error = true;
	return (1);
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
	this->_toParse.clear();
	this->_method.clear();
	this->_version.clear();
	this->_body.clear();
	this->_headers.clear();
	this->_multiHeaders.clear();
}

int isIncomplete(Request &obj, std::string &content)
{
	size_t crlf = content.find("\r\n\r\n");
	if (crlf == std::string::npos)
	{
		std::cout << "pas de CRLFCRLF dans isincomplete" << std::endl;
		return (1);
	size_t contentLen = obj.getContentLen();
	size_t contentLenCopy = obj.getContentLenCopy();
	if (obj.getTransferEncoding() && contentLenCopy != 0)
		return (2);
	else if (contentLen != 0 && contentLen != std::string::npos && contentLenCopy != 0 && contentLenCopy != std::string::npos)
		return (2);
	return (0);
}

/*
	La fonction detectBodyHeader sert à vérifier si un content-length
	ou un transfer-encoding est présent.
	Les deux headers ne sont pas compatibles et donc ne peuvent pas être ensemble.
	La fonction prévient les doublons et la compatibilité.
	Elle met aussi à jour les variables associées aux headers.
*/
int detectBodyHeader(Request &obj, std::string buffer)
{
	std::istringstream istream(buffer);
	std::string line;
	size_t pos;
	int res = 0;

	std::cout << "to parse dans body header: " << obj.getToParse() << std::endl;
	std::cout << "detect body header" << std::endl;
	while (std::getline(istream, line, '\n'))
	{
		//std::cout << "test de boucle" << std::endl;
		if (res >= 2)
			return (res);
		pos = line.find(":");
		if (pos == std::string::npos)
		{
			//std::cout << "pas de ':' dans line: " << line << std::endl;
			continue ;
		}
		//std::cout << "line: " << line << std::endl;
		std::string header(toLower(line, pos + 1));
		//std::cout << "header: " << header << std::endl;
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
			//std::cout << "res: " << res << std::endl;
		}
		else if (header.find("content-length:") != std::string::npos)
		{
			std::cout << "\e[0;31myo j'ai un content-lenght\e[0;m" << std::endl;
			size_t value;
			if (!(value_stream >> header >> value))
				obj.setContentLen(std::string::npos - 1);
			else
				obj.setContentLen(value);
			res++;
			std::cout << "res: " << res << std::endl;
			std::cout << "value: " << value << std::endl;
		}
	}
	std::cout << "tout est normal, on retourne " << res << std::endl; 
	return (res);
}

size_t countLenTransferEncoding(std::string buffer)
{
	std::istringstream iss(buffer);
	size_t len;

	if (!(iss >> std::hex >> len))
	{
		//std::cout << "iss a foiré" << std::endl;
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

size_t getBodyLen(std::string raw_buffer){
	std::istringstream iss(raw_buffer);
	std::string line;
	static std::string boundary;
	static bool inside_body;
	size_t len = 0;
	size_t colon = std::string::npos;

	if (boundary.empty()){
		inside_body = false;
		while (std::getline(iss, line, '\n')){
			colon = line.find(':');
			if (colon != std::string::npos){
				std::string key = toLower(line, colon);
				if (key != "content-type")
					continue ;
				std::string value = line.substr(colon + 1);
				size_t pos = value.find("boundary=");
				std::cout << "voici notre key: " << key << "notre value: " << value << std::endl;
				if (pos != std::string::npos){
					boundary = value.substr(pos + 9);
					std::cout << "on a trouvé le boundary: " << boundary << std::endl;
				}
				break ;
			}
		}
	}
	else if (boundary.empty())
	{
		std::cout << "boundary est vide" << std::endl;
		return 0;
	}
	size_t crlf = raw_buffer.find("\r\n\r\n");
	size_t pos = raw_buffer.find("boundary", crlf);
	if (pos != std::string::npos && !inside_body){
		inside_body = !inside_body;
		std::string body = raw_buffer.substr(pos);
		std::cout << "on sépare le body du raw buffer: " << body << std::endl;
		len += body.length();
		std::cout << "on trouve boundary une première fois" << std::endl;
	}
	else if (pos != std::string::npos && inside_body){
		len += raw_buffer.length();
		inside_body = !inside_body;
		boundary.clear();
		std::cout << "on trouve boundary une seconde fois" << std::endl;
	}
	else{
		len += raw_buffer.length();
		std::cout << "on ajoute simplement le bail" << std::endl;
	}
	std::cout << "on retourne len: " << len << std::endl;
	return len;
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
	if (detectBodyHeader(*this, buffer) >= 2 || this->_contentLen == std::string::npos - 1)
		return (assignError(makeError(400, "Bad Request for header")));
	std::cout << "detectBodyHeader est passé" << std::endl;
	std::cout << "content len:" << this->getContentLen() << std::endl;
	if ((this->_contentLen == std::string::npos && !this->getTransferEncoding()) && buffer.size() == 2 && ((int)buffer.at(0) == 13) && ((int)buffer.at(1) == 10))
	{
		this->_toParse.append(buffer);
		std::cout << "on retourne 1 | premiere condition" << std::endl;
		return (1);
	}
	//if (!checkCRLF(buffer))
	//{
	//	std::cout << "problème de CRLF" << std::endl;
	//	return (assignError(makeError(400, "Bad Request for CRLF")));
	//}
	//std::cout << "On check si complet" << std::endl;
	/*
		La première fois que cette fonction est appelé, _toParse est vide et on part
		du principe que le buffer contient une requête complète.
		Donc on l'assigne tout simplement.
	*/
	if (this->_toParse.empty())
		this->_toParse.assign(buffer);
	std::cout << "\e[0;31mon check ce que contient toParse:" << std::endl << this->_toParse << "\e[0;m" << std::endl;
	/*
		On vérifie que ce soit le cas avec la fonction isIncomplete(). Si elle est
		incomplète, on efface _toParse pour recommencer et faire les choses BIEN.
	*/
	if ((isHeader = isIncomplete(*this, this->_toParse)) > 0)
	{
		std::cout << "header incomplet" << std::endl;
		if (this->_toParse == buffer)
			this->_toParse.clear();
		//std::cout << "Requête pas complète" << std::endl;
		if (isHeader != 1 && this->getTransferEncoding())
		{
			//if (this->_toParse == buffer)
			//	this->_toParse.clear();
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
				{
					std::cout << "problème avec contentlencopy" << std::endl;
					return (assignError(makeError(400, "Bad Request for hexa")));
				}
			}
			this->_waitingForData = !this->_waitingForData;
			if ((!this->_waitingForData && buffer.size() - 2 < this->_contentLenCopy) || (this->_contentLenCopy != 0 && isRawEmpty(buffer)))
			{
				std::cout << "problème de buffer size" << std::endl;
				return (assignError(makeError(400, "Bad Request for buffer size")));
			}
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
				std::cout << "on retourne 1 | contentlencopy == 0" << std::endl;
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
			//std::cout << std::endl << "test : " << this->getContentLen() << this->getContentLenCopy() << std::endl;
			std::cout << "\e[0;31mj'attends\e[0;m" << std::endl;
			//this->_appendLen = buffer.length() - 2;
			this->_appendLen = getBodyLen(buffer);
			std::cout << buffer << std::endl;
			std::cout << "on veut ajouter " << this->_appendLen << " et contentlencopy == " << this->_contentLenCopy << std::endl;
			if (this->_contentLenCopy - this->_appendLen >= 0)
			{
				this->_contentLenCopy -= this->_appendLen;
				this->_toParse.append(buffer);
			}
			else
			{
				this->_toParse.append(buffer, 0, this->_contentLenCopy);
				std::cout << "on retourne 1 | contentlencopy = appendlen < 0" << std::endl;
				return (1);
			}
			if (this->_contentLenCopy == 0)
			{
				std::cout << "on retourne 1 | contentlencopy == 0" << std::endl;
				return (1);
			}
		}
		else
			this->_toParse.append(buffer);
		std::cout << "on retourne 0" << std::endl;
		return (0);
	}
	std::cout << "on retourne 1 fin de fonction" << std::endl;
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
	//std::cout << "-----------PARSE-----------" << std::endl;
	while (std::getline(iss, line, '\n'))
	{
		//std::cout << "Boucle pour check une ligne: " << line << std::endl;
		/*
			Recheck et suppression de CRLF
		*/
		//std::cout << "\e[0;31m line: " << line << "\e[0;m" << std::endl;
		if (!isBody && line.at(line.size() - 1) != 13)
		{
			std::cout << "body: " << this->getBody() << std::endl;
			return ((void)assignError(makeError(400, "Bad Request CRLF in parse()")));
		}
		line.erase(line.size() - 1);
		//std::cout << "apres check CRLF" << std::endl;
		std::istringstream issLine(line);
		if (i == 0)
		{
			/*
				Parsing de la request line (methode, fichier, version)
			*/
			//std::cout << "i == 0 donc on passe dans la condition" << std::endl;
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
			//std::cout << "line est pas empty et isBody est faux donc on passe dans la condition" << std::endl;
			if (line.size() > MAX_REQUEST_LINE_SIZE)
				return ((void)assignError(makeError(413, "Content Too Large")));
			//std::cout << "line size est ok" << std::endl;
			if (std::isspace(line.at(0)))
			{
				//std::cout << "issapce a 0" << std::endl;
				this->_headers[currentKey].append(line);
			}
			else
			{
				//std::cout << "pas de space a 0" << std::endl;
				size_t colon = line.find(':');
				//std::cout << "on a find colon";
				if (colon == std::string::npos)
				{
					//std::cout << " mais il est == a npos" << std::endl;
					return ((void)assignError(makeError(400, "Bad Request pas de ':'")));
				}
				//std::cout << ": " << colon << std::endl;
				std::string key = toLower(line, colon);
				currentKey = key;
				//std::cout << "on a la currentKey: " << currentKey << std::endl;
				//std::string value = ft_strtrim(line.substr(colon + 1));
				std::string value = line.substr(colon + 1);
				//std::cout << "value a subit le substr: " << value << std::endl;
				std::map<std::string, std::string>::iterator it;
				if (key == "authorization" || key == "proxy-authorization")
				{
					//std::cout << "key == authorization ou proxy-authorization" << std::endl;
					this->_multiHeaders.insert(std::pair<std::string, std::string>(key, value));
				}
				else if ((it = this->_headers.find(key)) == this->_headers.end())
				{
					//std::cout << "key == end?" << std::endl;
					this->_headers[key] = value;
					//std::cout << "segfault ici?" << std::endl;
				}
				else if (!uniqueHeaders.count(key))
				{
					//std::cout << "header doit etre unique" << std::endl;
					it->second.append(", " + value);
				}
				else
				{
					std::cout << "\e[0;32m" << key << ' ' << value << "\e[0;m" << std::endl;
					return ((void)assignError(makeError(400, "Bad Request multiple unique header")));
				}
			}
		}
		else
		{
			//std::cout << "Aucune condition n'est vrai alors on append" << std::endl;
			if (!isBody)
				isBody = true;
			this->_body.append(line);
		}
		i++;
	}
	//std::cout << "Fin de la boucle" << std::endl;
	if (!this->_headers.count("host"))
		return ((void)assignError(makeError(400, "Bad Request pas de host")));
	if (this->_headers.size() > MAX_HEADERS_SIZE)
		return ((void)assignError(makeError(413, "Content Too Large")));
//	std::cout << "method:	" << this->_method << std::endl;
//	std::cout << "content:	" << this->_content << std::endl;
//	std::cout << "version:	" << this->_version << std::endl;
//	std::cout << "map:" << std::endl;
//	for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
//	{
//		std::cout << "first:	" << it->first << ", ";
//		std::cout << "second:	" << it->second << std::endl;
//	}
//	std::cout << "multimap:" << std::endl;
//	for (std::multimap<std::string, std::string>::iterator it = this->_multiHeaders.begin(); it != this->_multiHeaders.end(); it++)
//	{
//		std::cout << "first:	" << it->first << ", ";
//		std::cout << "second:	" << it->second << std::endl;
//	}
//	std::cout << "Body: " << this->_body << std::endl;
}
