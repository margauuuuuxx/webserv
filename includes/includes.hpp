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

#include "Client.hpp"
#include "Config.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "utils.hpp"

# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define RESET "\033[0m"
#define MAX_REQUEST_SIZE 16384

char	*ft_itoa(int n);
