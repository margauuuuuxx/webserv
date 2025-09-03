#include "../includes/includes.hpp"

/*
	This class is responsible for halding pointers to all the Socket objects created by the server.
*/

SocketArray::SocketArray(){}
SocketArray::~SocketArray(){
	std::cout << "destructor of the erray" << std::endl;
	for (size_t i = 0; i < _sockets.size() ; i++) {
		delete _sockets[i];
	}
}
void SocketArray::pushBack(Socket* s){
	_sockets.pushBack(s);
}
Socket* SocketArray::operator [] (size_t i){
	return _sockets[i];
}
size_t SocketArray::size() const{
	return _sockets.size();
}
