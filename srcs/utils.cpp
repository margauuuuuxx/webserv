#include "../includes/includes.hpp"

void    error_exit(std::string message)
{
    std::cerr << RED << "ERROR EXIT: " << message << std::endl;
    exit(EXIT_FAILURE);
}