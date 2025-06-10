#pragma once 

#include "includes.hpp"

/*
    This class is responsible for parsing HTTPS requests
*/

class Request
{
    private:
        Request();
        Request(const Request &other);
        Request& operator=(const Request &other);

    public:
        Request(const std::string &input);
        ~Request();
};