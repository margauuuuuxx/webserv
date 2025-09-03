#include "../includes/includes.hpp"

void Response::buildErrorResponse(int code, Request& req, Server& server) {
    this->_statusCode = code;
    this->_httpVersion = req.getVersion();

    // Check for a custom error page in the server config.
    std::map<int, std::string>::const_iterator it = server.errorPages.find(code);
    if (it != server.errorPages.end()) { // CHECK FOR ERROR PAGES 
        if (readFile(it->second, this->_content)) {
            this->_contentSize = this->_content.size();
            return;
        }
    }

    // If no custom page, create a default one.
    std::ostringstream oss;
    oss << "<html>\n<head><title>" << this->_statusCode << " " << statusMessages[this->_statusCode]
        << "</title></head>\n<body>\n<h1>" << this->_statusCode << " " << statusMessages[this->_statusCode]
        << "</h1>\n</body>\n</html>";
    this->_content = oss.str();
    this->_contentSize = this->_content.size();
}

Route*  Response::findRoute(Request& req, Server &server)
{
    std::string location = req.getContent();
    for (size_t i = 0; i < server.routes.size(); ++i) {
        if (server.routes[i].location == location)
            return &server.routes[i];
    }
    return (NULL);
}

