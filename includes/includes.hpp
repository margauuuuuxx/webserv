#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <exception>
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
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define RESET "\033[0m"
# define MAX_REQUEST_SIZE 1048576 // max nmbr of bytes the server will read from a socket in a single recv call 
# define MAX_REQUEST_LINE_SIZE 8192
# define MAX_HEADERS_SIZE 65536
# define MAX_BODY_SIZE 974848
# define TIMEOUT_SECONDS 5

#ifdef DEBUG_MODE
    #define DEBUG_LOG(msg) std::cerr << msg << std::endl
#else 
    #define DEBUG_LOG(msg)
#endif

#include "CGI.hpp"
#include "CGIutils.hpp"
#include "Parser.hpp"
#include "Poller.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "responseUtils.hpp"
#include "Server.hpp"
#include "Socket.hpp"
#include "SocketArray.hpp"
#include "utils.hpp"
