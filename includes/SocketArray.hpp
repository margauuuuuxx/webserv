#pragma once

#include "includes.hpp"

class SocketArray{
public:
	SocketArray();
	~SocketArray();
	
	void	push_back(Socket* s);
	Socket*	operator [] (size_t i);
	size_t	size() const;
	
private:
	std::vector<Socket*> _sockets;
};

