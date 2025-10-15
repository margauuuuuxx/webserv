#include "../includes/includes.hpp"

void    errorExit(std::string message)
{
    std::cerr << RED << "ERROR EXIT: " << message << std::endl;
    exit(EXIT_FAILURE);
}

std::string loadFile(const std::string &path)
{
    std::ifstream   file(path.c_str());

    if (!file.is_open())
        return ("");

    std::stringstream   buffer;
    buffer << file.rdbuf();
    return (buffer.str());
}

std::string intToString(const std::string &str)
{
    std::stringstream   ss;

    ss << str.length();
    return (ss.str());
}

static int	count_char(long a)
{
	long	i;
	int		count;

	i = 1;
	count = 0;
	if (a < 0)
	{
		a *= -1;
		count++;
	}
	if (a == 0)
		count++;
	while (i <= a)
	{
		i *= 10;
		count++;
	}
	return (count);
}

char	*ftItoa(int n)
{
	char	*num;
	int		i;
	long	a;

	a = n;
	i = count_char(a);
	num = new char[i + 1];
	if (!num)
		return (NULL);
	if (a < 0)
	{
		a *= -1;
		num[0] = '-';
	}
	num[i] = '\0';
	i--;
	while (a >= 10)
	{
		num[i] = (a % 10) + '0';
		a /= 10;
		i--;
	}
	if (a <= 9)
		num[i] = a + '0';
	return (num);
}

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
		//std::cout << "on a testé: ";
		//for (size_t yo = temp; yo <= i; yo++){
		//	std::cout << s[yo];
		//}
		//std::cout << std::endl;
		//std::cout << "on a trouvé une string qui correspond" << std::endl;
		//std::cout << "temp: " << temp << std::endl;
		//std::cout << "i: " << i << std::endl;
		//std::cout << "j: " << j << std::endl;
		//std::cout << "strlen: " << std::strlen(str) << std::endl;
		if (j == std::strlen(str))
		{
			//std::cout << "on retourne temp" << std::endl;
			return temp;
		}
	}
	//std::cout << "on retourne npos" << std::endl;
	return std::string::npos;
}
