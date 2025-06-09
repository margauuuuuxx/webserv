#ifndef SERVERCONFIG
# define SERVERCONFIG

#include <string>
#include <vector>
#include <map>

struct LocationConfig {
    std::string path;
    std::string root;
    std::string index;
    std::vector<std::string> allow_methods;
    bool autoindex;
    std::string upload_store;
    std::string cgi_extension;
    std::string cgi_path;
    std::string redirect_url;
    int redirect_code;
    
    LocationConfig() : autoindex(false), redirect_code(0) {}
};

class ServerConfig{
public:
	ServerConfig();
	~ServerConfig();
	void getPort(const std::vector<std::string>& tokens, size_t& i);
	void getHost(const std::vector<std::string>& tokens, size_t& i);
	void getErrorPage(const std::vector<std::string>& tokens, size_t& i);
	void getClientMaxBodySize(const std::vector<std::string>& tokens, size_t& i);
	void getLocations(const std::vector<std::string>& tokens, size_t& i);
private:
	int port;
	std::string host;
	std::vector<std::string> server_names;
	std::map<int, std::string> error_pages;
	size_t client_max_body_size;
	std::vector<LocationConfig> locations;
};

#endif // !serverconfig
