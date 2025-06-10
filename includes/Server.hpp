#pragma once 

#include "includes.hpp"

/*
    Class responsible for :
        - listening on a port
        - accepting new clients
        - holding configuration and client list
*/

class Server
{
    private:
        Server(const Server &other);
        Server& operator=(const Server &other);

    public:
        Server();
        ~Server();
};