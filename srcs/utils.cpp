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

char	*ft_itoa(int n)
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

std::string ft_strtrim(std::string s)
{
	size_t start = 0;
	std::string::iterator it = s.begin();
	while (std::isspace(*it))
	{
		start++;
		it++;
	}
	size_t len = 0;
	while (!std::isspace(*it))
	{
		len++;
		it++;
	}
	return (s.substr(start, len));
}
