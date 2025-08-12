#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h> // for specific sockaddr strcuts 
#include <poll.h>
#include <set>
#include <signal.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define RESET "\033[0m"
# define MAX_REQUEST_SIZE 1048576
# define MAX_REQUEST_LINE_SIZE 8192
# define MAX_HEADERS_SIZE 65536

#include "Client.hpp"
#include "Config.hpp"
#include "Parser.hpp"
#include "Poller.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Socket.hpp"
#include "SocketErray.hpp"
#include "utils.hpp"
