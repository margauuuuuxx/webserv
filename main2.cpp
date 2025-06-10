#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <signal.h>
#include <cstdlib>
#include <fstream>

void ft_error(std::string message)
{
	std::cerr << message << std::endl;
	exit(EXIT_FAILURE);
}

std::string whichPage(char buffer[10000])
{
	int i = 0;
	std::string res;

	std::cout << buffer << std::endl;
	while (buffer[i])
	{
		if (buffer[i] == '/' && buffer[i + 1])
		{
			i++;
			break ;
		}
		i++;
	}
	while (buffer[i] != ' ' || buffer[i] != '\n')
	{
		if (buffer[i] == ' ' || buffer[i] == '\n')
			break ;
		res.push_back(buffer[i]);
		i++;
	}
	return (res);
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

std::string getPage(std::string page)
{
	std::ifstream infile;
	std::string res;
	std::string content;
	std::string line;

	res.clear();
	if (page.empty())
	{
		res.append("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
		return (res);
	}

	//std::string full_path = "srcs/" + page;

	infile.open(page.c_str());
	if (!infile.is_open())
	{
		res.append("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
		return (res);
	}
	while (getline(infile, line))
	{
		content.append(line);
	}
	res.append("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: ");
	char *number = ft_itoa(content.length());
	delete number;
	res.append("\r\n\r\n");
	res.append(content);
	return (res);
}

int main(void)
{
	int			sockfd;
	const int	port = 8080;
	sockaddr_in	address;
	sockaddr_in	client_address;
	int			new_socket;
	socklen_t	addrlen = sizeof(address);
	socklen_t	client_addrlen = sizeof(client_address);
	std::string	hello;// = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
	char		buffer[10000] = {0};
	int			valread;

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		ft_error("Socket failed");
	memset(&address, 0, sizeof(address));
	address.sin_family = PF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	int reuse = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
		ft_error("setsockopt(SO_REUSEADDR) failed");

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
		ft_error("setsockopt(SO_REUSEPORT) failed");


	if (bind(sockfd, (struct sockaddr *)&address, addrlen) < 0)
		ft_error("Bind failed");
	if (listen(sockfd, 10) < 0)
		ft_error("Listen failed");

	while (1)
	{
		std::string page;
		if ((new_socket = accept(sockfd, (struct sockaddr *)&client_address, &client_addrlen)) < 0)
			ft_error("accept failed");
		
		valread = read(new_socket, buffer, 10000);
		if(valread < 0)
			ft_error("No bytes are there to read");
		if (valread == 0)
			std::cout << "Client disconnected" << std::endl;
		else
			page = whichPage(buffer);
		hello = getPage(page);
		if (hello.empty())
		{
			std::cerr << "Page does not exist" << std::endl;
			continue ;
		}
		write(new_socket , hello.c_str() , hello.length());
		close(new_socket);
	}
	return (0);
}
