#pragma once

#include "includes.hpp"

void        errorExit(std::string message);
std::string loadFile(const std::string &path);
std::string intToString(const std::string &str);
char	    *ft_itoa(int n);
std::string makeError(int code, const std::string& message);
std::string toLower(std::string line, size_t end);
std::string ft_strtrim(std::string s);