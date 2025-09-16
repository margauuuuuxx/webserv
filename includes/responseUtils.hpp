#pragma once 

#include "includes.hpp"

struct Route;

bool    readFile(const std::string& path, std::string& content);
bool    isCGIReq(const std::string& resource, const Route* route);
