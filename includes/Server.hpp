#pragma once

#include "includes.hpp"

class Server
{
	private:
		int	_sockfd;
		sockaddr_in	_address;
		int	_port;
		std::vector<struct pollfd> _pollFd;
	public:
		Server(void);
		~Server(void);

		void	setup(void);
		void	run(void);
};
