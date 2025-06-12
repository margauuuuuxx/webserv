#include "../includes/Server.hpp"

Server::Server() 
{
    _sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (_sockfd < 0)
        errorExit("Failed to create socket");

    _port = 8080;
    memset(&_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;
    _address.sin_port = htons(_port);
    _address.sin_addr.s_addr = htonl(INADDR_ANY);
    _addrlen = sizeof(_address);
    
    int reuse = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        errorExit("Function setsockopt failed");

    if (bind(_sockfd, (const struct sockaddr *) &_address, sizeof(_address)) < 0)
        errorExit("Failed to bind");

    if (listen(_sockfd, 10) < 0) // 10 is arbitrary # --> NEEDS TO CHANGE
        errorExit("Failed to listen for sockfd = " + _sockfd);

    while (1)
    {
        char buffer[10000] = {0};

        int newSocket = accept(_sockfd, (struct sockaddr *) &_address, &_addrlen);
        if (newSocket < 0)
            errorExit("Accept function failed for socket fd = " + newSocket);
    
        int bytesRead = read(newSocket, buffer, 10000);
        if (bytesRead < 0)
            errorExit("No bytes to read");
        if (bytesRead == 0)
            errorExit("No bytes read: Client disconnected");
    
        std::string pageContent = loadFile("index.html");
        std::cout << "Page content size = " << pageContent.length() << std::endl;
        std::string header = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: " + intToString(pageContent) + "\r\n\r\n";
        std::string response = header + pageContent;

        std::cout << header << std::endl;

        write(newSocket, response.c_str(), response.length());
        close(newSocket);
    }
}

Server::~Server() {}
