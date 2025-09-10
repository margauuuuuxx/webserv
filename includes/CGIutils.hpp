#pragma once

#include "includes.hpp"

class Response;

std::vector<std::string> getSNandPI(const std::string& scriptPath, const std::string& reqURL);
std::string getQueryString(const std::string& reqURL);
void    getHeaders(std::vector<std::string>& envVector, Request &req);
char**  vectToArray(const std::vector<std::string>& v);
void    closePipes(int pipe1[2], int pipe2[2]);
void    freeEnvv(char **envv);
std::string readCGI(int fd);