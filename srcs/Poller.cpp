#include "../includes/includes.hpp"

/*
    This class is responsible for the I/O multiplexing of the server.
    It wraps the poll() system call and holds a list of file descriptors to monitor.
    The server will use this class to wait for activity on any of the sockets.
*/

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
void Poller::modifyFd(int fd, short events) {
    for (size_t i = 0; i < _fds.size(); ++i) {
        if (_fds[i].fd == fd) {
            _fds[i].events = events; // change les événements surveillés
            return;
        }
    }
    throw std::runtime_error("modifyFd: FD not found");
}
