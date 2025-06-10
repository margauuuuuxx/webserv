#include "../includes/Server.hpp"

Server::Server() 
{
        // PF_INET = domain --> protocol family: socket used for communication
        // SOCK_STREAM      --> protocol type: SOCK-STREAM is for 2-way reliable communication
        // 0                --> protocol argument: 0 is the default config  
    if (this->sockfd = socket(PF_INET, SOCK_STREAM, 0) < 0)
        error_exit("Failed to create socket");
        
}

Server::~Server() {}
