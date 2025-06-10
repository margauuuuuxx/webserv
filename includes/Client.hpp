#pragma once 

#include "includes.hpp"

/*
    Class responsible for:
        - having a socket
        - storing the incoming buffer
        - managing the socket states (R-W-C)
*/

class Client
{
    private:
        Client(const Client &other);
        Client& operator=(const Client &other);

    public:
        Client();
        ~Client();
};