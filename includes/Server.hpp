#pragma once 

#include "includes.hpp"

/*
    Class responsible for :
        - listening on a port
        - accepting new clients
        - holding configuration and client list
*/

static const int    BACKLOG = 10;
static const int    BUFFER_SIZE = 10000;

class Server
{
    private:
        Server(const Server &other);
        Server& operator=(const Server &other);
        
        void        run();

        int         _sockfd;
        uint16_t    _port;
        sockaddr_in _address;
        socklen_t   _addrlen;

    public:
        Server();
        ~Server();
};