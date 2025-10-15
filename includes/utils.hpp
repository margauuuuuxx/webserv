#pragma once

#include "includes.hpp"

void        errorExit(std::string message);
std::string loadFile(const std::string &path);
std::string intToString(const std::string &str);
char	    *ftItoa(int n);
std::string makeError(int code, const std::string& message);
std::string toLower(std::string line, size_t end);
std::string ftStrtrim(std::string s);
size_t		ft_strchr(const char *s, const char *str, int bytes, size_t pos);
