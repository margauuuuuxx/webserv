#include "../includes/Server.hpp"

Server::Server() 
{
    try {
        // PF_INET = domain --> protocol family: socket used for communication
        // SOCK_STREAM      --> protocol type: SOCK-STREAM is for 2-way reliable communication
        // 0                --> protocol argument: 0 is the default config  
        this->sockfd = socket(PF_INET, SOCK_STREAM, 0);

    } catch(const std::exception &e) {
        std::cerr << RED << e.what() << RESET << std::endl;
    }
}

Server::~Server() {}
