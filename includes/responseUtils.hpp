#pragma once 

#include "includes.hpp"

bool readFile(const std::string& path, std::string& content);
bool    isFile(const std::string& path);
bool    isDir(const std::string& path);
bool    isCGIReq(const std::string& resource, const Route* route);
