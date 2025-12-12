#pragma once

#include "includes.hpp"

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
};
