#include "Server.hpp"
#include <map>
#include <unordered_map>

class ServerManager{
public:
	ServerManager();
	~ServerManager();
	void init_server(std::vector<Server> servers);
private:
	std::unordered_map<int, Server> _servers;
};
