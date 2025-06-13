#pragma once 

#include "includes.hpp"

/*
    This class is responsible for hoding the server config
*/

class Config
{
    private:
        Config(const Config &other);
        Config& operator=(const Config &other);

    public:
        Config();
        ~Config();
};