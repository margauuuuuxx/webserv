#include "../includes/ServerManager.hpp"
#include <vector>

ServerManager::ServerManager(){}
ServerManager::~ServerManager(){}
void ServerManager::init_server(std::vector<Server> servers){
	for (std::vector<Server>::iterator i = servers.begin(); i != servers.end(); i++) {
		this->_servers.insert(std::pair<int, Server>(1, *i));	
	}
}
