#pragma once

#include <sys/select.h>
#include <sys/socket.h>
#include <iostream>
#include <stdexcept>
#include <netinet/in.h> // for specific sockaddr strcuts 
#include <cstring>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <arpa/inet.h>
#include <signal.h>
#include <vector>
#include <map>
#include <poll.h>
#include <ctime>
#include <set>

# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define RESET "\033[0m"
# define MAX_REQUEST_SIZE 1048576
# define MAX_REQUEST_LINE_SIZE 8192
# define MAX_HEADERS_SIZE 65536

#include "Client.hpp"
#include "Config.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "utils.hpp"

char	*ft_itoa(int n);
std::string makeError(int code, const std::string& message);
std::string toLower(std::string line, size_t end);
