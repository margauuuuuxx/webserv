#include "../includes/includes.hpp"

std::string loadFile(const std::string &path)
{
    std::ifstream   file(path.c_str());

    if (!file.is_open())
        return ("");

    std::stringstream   buffer;
    buffer << file.rdbuf();
    return (buffer.str());
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

std::string ftStrtrim(std::string s)
{
	size_t start = 0;
	std::string::iterator it = s.begin();
	while (it != s.end() && std::isspace(*it))
	{
		start++;
		it++;
	}
	size_t len = 0;
	while (it != s.end() && !std::isspace(*it))
	{
		len++;
		it++;
	}
	return (s.substr(start, len));
}

size_t ft_strchr(const char *s, const char *str, int bytes, size_t pos)
{
	size_t max = static_cast<size_t>(bytes);
	//std::cout << "\e[0;31mmax: " << max << std::endl;
	//std::cout << "pos: " << pos << "\e[0;m" << std::endl;
	if (pos == std::string::npos || pos >= max)
	{
		//std::cout << "pos est pas bon: " << pos << std::endl;
		return std::string::npos;
	}
	size_t temp;
	for (size_t i = pos; i < max; i++){
		size_t j = 0;
		if (s[i] != str[j])
			continue ;
		temp = i;
		while (i < max && str[j] && s[i] == str[j]){
			i++;
			j++;
		}
		if (j == std::strlen(str))
			return temp;
	}

	return std::string::npos;
}

std::string	intToString(int num) {
	std::stringstream ss;
	ss << num;
	return (ss.str());
}

std::vector<char>	generateErrorResponse(int code, const std::string& statusMessage) {
	std::string body = "<!DOCTYPE html><html lang=\"en\"><head><title>" + intToString(code) + " " +
						statusMessage + "</title></head>" + "<body><hi>" + intToString(code) + " " +
						statusMessage + "</h1>" + 
						"<p>The server could not understand the request due to the malformed syntax.</p></body></html>";

	std::stringstream res;
	res << "HTTP/1.1 " << code << " " << statusMessage << "\r\n";
   	res << "Content-Type: text/html\r\n";
   	res << "Content-Length: " << body.length() << "\r\n";
   	res << "Connection: close\r\n";
   	res << "\r\n";
   	res << body;

	std::string resStr = res.str();
	return (std::vector<char>(resStr.begin(), resStr.end()));
}
