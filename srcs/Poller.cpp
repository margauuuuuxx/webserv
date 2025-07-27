#include "../includes/Poller.hpp"
#include <unistd.h>
#include <stdexcept>
#include <algorithm>

Poller::Poller() {}

Poller::~Poller() {
    for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it)
        close(it->fd);
}

void Poller::addFd(int fd, short events) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;
    _fds.push_back(pfd);
}

void Poller::removeFd(int fd) {
    for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it) {
        if (it->fd == fd) {
            close(it->fd);
            _fds.erase(it);
            break;
        }
    }
}

int Poller::wait(int timeout) {
    int ret = poll(&_fds[0], _fds.size(), timeout);
    if (ret < 0)
        throw std::runtime_error("poll() failed");
    return ret;
}

std::vector<struct pollfd>& Poller::getFds() {
    return _fds;
}
