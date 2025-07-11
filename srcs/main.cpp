#include "../includes/Parser.hpp"
#include "../includes/ServerManager.hpp"

int main (int argc, char *argv[]) {
	Parser p;
	ServerManager manager;

	if(argc == 2)
		p.parsefile(argv[1]);
	else
		return std::cerr << "Error: wrong number or args" << std::endl, 1;

	return 0;
}
