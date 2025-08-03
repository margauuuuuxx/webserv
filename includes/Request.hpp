#pragma once
#include <string>

class Request{
public:
	Request();
	~Request();
	void addPart(char *str);
	bool isReady();

private:
	std::string _req;
};
