#ifndef POLLER_HPP
#define POLLER_HPP

#include <vector>
#include <poll.h>

class Poller {
public:
    Poller();
    ~Poller();

    void addFd(int fd, short events);
    void removeFd(int fd);
    int wait(int timeout);
	void modifyFd(int fd, short events);
    std::vector<struct pollfd>& getFds();

private:
    std::vector<struct pollfd> _fds;

    Poller(const Poller&);
    Poller& operator=(const Poller&);
};

#endif

