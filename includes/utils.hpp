#pragma once

#include "includes.hpp"

std::string         loadFile(const std::string &path);
std::string         toLower(std::string line, size_t end);
std::string         ftStrtrim(std::string s);
size_t		        ft_strchr(const char *s, const char *str, int bytes, size_t pos);
std::string         intToString(int num);
std::vector<char>   generateErrorResponse(int code, const std::string& statusMessage);
