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

        int         _sockfd;
        int         _port;
        sockaddr_in _address;
        socklen_t   _addrlen;

    public:
        Server();
        ~Server();
};