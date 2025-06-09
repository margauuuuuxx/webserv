
#include "../includes/Parser.hpp"
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>
Parser::Parser(){}
Parser::~Parser(){}
void Parser::parsefile(const std::string& filename){
	std::ifstream file;
	try{
		file = openfile(filename);
		tokenizer(file);
		// printTokens();

	}catch(std::exception& e){
		std::cout << "Parsing error: " << e.what() << std::endl;
	}
}

void Parser::tokenizer(std::ifstream& file){
	 std::string line;

	while (std::getline(file, line)){
		cleanComments(line);
		for (size_t i = 0; i < line.size(); i++){
			if (line[i] == '{' || line[i] == '}' || line[i] == ';'){
				line.insert(i, " "); 
				i += 2; 
				line.insert(i, " "); 
				i++;
			}
		}
		std::istringstream iss(line);
		std::string token;
		while (iss >> token) {
			_tokens.push_back(token);
		}
	}
}

void Parser::cleanComments(std::string& line) const {
	size_t pos = line.find("#");
	if (pos != std::string::npos) {
		line = line.substr(0, pos);
	}
}

std::ifstream Parser::openfile(const std::string& filename){
	std::ifstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Cannot open config file: " + filename);
	}
	return file;
}
void Parser::printTokens() const {
    for (size_t i = 0; i < _tokens.size(); i++) {
        std::cout << "[" << i << "] '" << _tokens[i] << "'" << std::endl;
    }
    std::cout << "Total _tokens: " << _tokens.size() << std::endl;
}
void Parser::parser(){
	_i = 0;
	while (_i < _tokens.size()){
		if (_tokens[_i] == "server") 
		{
			parseServer();
		}
	}
}
void Parser::parseServer(){

}
