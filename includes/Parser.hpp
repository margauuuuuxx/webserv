#ifndef PARSER
# define PARSER

#include <vector>
#include <iostream>
#include <exception>
#include <fstream>
#include <sstream>
#include "ServerConfig.hpp"

class Parser{
public:
	Parser();
	~Parser();
	void parsefile(const std::string& filename);
private:
	std::vector<std::string>	_tokens;
	size_t						_i;

	void tokenizer(std::ifstream& file);
	std::ifstream openfile(const std::string& filename);
	void cleanComments(std::string& line) const;
	void printTokens() const;
	void parser();
	void parseServer();
};

#endif // !PARSER
