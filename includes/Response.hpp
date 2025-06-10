#pragma once 

#include "includes.hpp"
#include "Request.hpp"
#include "Config.hpp"

/*
    This class is responsible for generating HTTPS responses :
        - based on requests and server state
        - handles file serving/errors/headings
*/

class Response
{
    private:
        Response();
        Response(const Response &other);
        Response& operator=(const Response &other);

    public:
        Response(const Request& req, const Config& conf);
        ~Response();
};