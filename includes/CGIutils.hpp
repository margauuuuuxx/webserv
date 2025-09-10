#pragma once

#include "includes.hpp"

std::vector<std::string> getSNandPI(const std::string& scriptPath, const std::string& reqURL);
std::string getQueryString(const std::string& reqURL);
void    getHeaders(std::vector<std::string>& envVector, Request &req);
char**  vectToArray(const std::vector<std::string>& v);