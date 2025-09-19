#include "../includes/SocketErray.hpp"
SocketErray::SocketErray(){}
SocketErray::~SocketErray(){
	std::cout << "destructor of the erray" << std::endl;
	for (size_t i = 0; i < _sockets.size() ; i++) {
		delete _sockets[i];
	}
}
void SocketErray::push_back(Socket* s){
	_sockets.push_back(s);
}
Socket* SocketErray::operator [] (size_t i){
	return _sockets[i];
}
size_t SocketErray::size() const{
	return _sockets.size();
}
