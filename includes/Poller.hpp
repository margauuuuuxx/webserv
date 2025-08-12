#pragma once

#include "includes.hpp"

class Poller {
public:
    Poller();
    ~Poller();

    void addFd(int fd, short events);
    void removeFd(int fd);
    int wait(int timeout);
    std::vector<struct pollfd>& getFds();

private:
    std::vector<struct pollfd> _fds;

    Poller(const Poller&);
    Poller& operator=(const Poller&);
};
