#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class Socket {
public:
	Socket(int port);
	~Socket();
	int getFd() const;
	int clientConnect();

private:
	int sockfd;
	int listenSocketFd;
	sockaddr_in addr;
	sockaddr_in clientaddr;
	socklen_t addrlen;
	socklen_t clientaddrlen;
	int backlog;
	
};
