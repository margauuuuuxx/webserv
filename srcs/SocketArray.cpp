#include "../includes/includes.hpp"

/*
	This class is responsible for halding pointers to all the Socket objects created by the server.
*/

SocketArray::SocketArray(){}
SocketArray::~SocketArray(){
	std::cout << "destructor of the array" << std::endl;
	for (size_t i = 0; i < _sockets.size() ; i++) {
		delete _sockets[i];
	}
}
void SocketArray::push_back(Socket* s){
	_sockets.push_back(s);
}
Socket* SocketArray::operator [] (size_t i){
	return _sockets[i];
}
size_t SocketArray::size() const{
	return _sockets.size();
}
