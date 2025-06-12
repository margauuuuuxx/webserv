#include "../includes/Server.hpp"

Server::Server() : _port(8080), _addrlen(sizeof(_address))
{
    _sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (_sockfd < 0)
        errorExit("Failed to create socket");

    int reuse = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        errorExit("Function setsockopt failed");

    memset(&_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;
    _address.sin_port = htons(_port);
    _address.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(_sockfd, (const struct sockaddr *) &_address, sizeof(_address)) < 0)
        errorExit("Failed to bind");

    if (listen(_sockfd, BACKLOG) < 0)
        errorExit("Listen function failed");

    run();
}

Server::~Server() {}

void    Server::run()
{
    char buffer[BUFFER_SIZE];
    
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        int newSocket = accept(_sockfd, (struct sockaddr *) &_address, &_addrlen);
        if (newSocket < 0)
            errorExit("Accept function failed");
    
        int bytesRead = read(newSocket, buffer, BUFFER_SIZE);
        if (bytesRead < 0)
        {
            close(newSocket);
            errorExit("No bytes to read");
        }
        if (bytesRead == 0)
        {
            close(newSocket);
            errorExit("No bytes read: Client disconnected");
        }
    
        std::string pageContent = loadFile("index.html");
        std::cout << "Page content size = " << pageContent.length() << std::endl;
        std::string header = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: " + intToString(pageContent) + "\r\n\r\n";
        std::string response = header + pageContent;

        ssize_t sent = write(newSocket, response.c_str(), response.length());
        if (sent < 0)
        {
            close(newSocket);
            errorExit("Failed to write to client socket");
        }

        close(newSocket);
    }
}
