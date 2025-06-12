#include "../includes/includes.hpp"

void    errorExit(std::string message)
{
    std::cerr << RED << "ERROR EXIT: " << message << std::endl;
    exit(EXIT_FAILURE);
}

std::string loadFile(const std::string &path)
{
    std::ifstream   file(path.c_str());

    if (!file.is_open())
        return ("");

    std::stringstream   buffer;
    buffer << file.rdbuf();
    return (buffer.str());
}

std::string intToString(const std::string &str)
{
    std::stringstream   ss;

    ss << str.length();
    return (ss.str());
}