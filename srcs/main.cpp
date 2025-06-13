#include "../includes/includes.hpp"

int main(void)
{
	Server server;

	try
	{
		server.setup();
		server.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return (0);
}