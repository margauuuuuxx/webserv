#pragma once

#include "webserv.hpp"

class WebServer
{
	private:
		int	_sockfd;
		sockaddr_in	_address;
		int	_port;
	public:
		WebServer(void);
		~WebServer(void);

		void	setup(void);
		void	run(void);
};
