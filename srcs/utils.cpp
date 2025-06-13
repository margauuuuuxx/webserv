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

