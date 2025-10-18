#include "../includes/includes.hpp"

CGI::CGI(Request &req, Server& server, const std::string& scriptPath, Route* route) : _req(req), _server(server), _scriptPath(scriptPath), _envv(NULL), _pid(-1), _pipe_out_fd(-1), _route(route) {
	DEBUG_LOG(GREEN << "CGI constructor: route=" << route << ", cgiPath=" << (route ? route->cgiPath : "NULL") << RESET);
	_reqURL = req.getContent();
	DEBUG_LOG(GREEN << "CGI constructor: calling _setEnvv()" << RESET);
	_setEnvv();
	DEBUG_LOG(GREEN << "CGI constructor: _setEnvv() complete" << RESET);
}

CGI::~CGI() {
	if (_envv != NULL) {
		for (size_t i = 0; _envv[i] != NULL; ++i)
			free (_envv[i]);
		delete[] _envv;
	}
}

void	CGI::_setEnvv() {
	std::vector<std::string> envVector;

	envVector.push_back("SERVER_SOFTWARE=webserv/1.0");
	envVector.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envVector.push_back("SERVER_PROTOCOL=" + _req.getVersion());
	envVector.push_back("REQUEST_METHOD=" + _req.getMethod());
	envVector.push_back("SCRIPT_FILENAME=" + _scriptPath);
	envVector.push_back("CONTENT_LENGTH=" + intToString(_req.getContentLen()));
	envVector.push_back("SERVER_NAME=" + _server.host);
	envVector.push_back("SERVER_PORT=" + intToString(_server.port));
	envVector.push_back("REMOTE_ADDR=" + _req.getClientIP());  

	_setQueryString();
	envVector.push_back("QUERY_STRING=" + _queryString);

	_setSNandPI();
	envVector.push_back("SCRIPT_NAME=" + _scriptName);
	envVector.push_back("PATH_INFO=" + _pathInfo);

	_setHeaders(envVector);

	_vectToArray(envVector);
}

// The goal of tis function is to create a child process that will transform into the CGI script
pid_t	CGI::execute() {
	DEBUG_LOG(YELLOW << "CGI::execute() started" << RESET);
	int pipe_in[2]; // sending data to the script
	int pipe_out[2]; // getting data from the script 

	if (pipe(pipe_in) == -1) {
		DEBUG_LOG(RED << "Error: " << RESET << "handleCGI: pipe() failed for pipe_in");
		return (-1);
	}
	DEBUG_LOG(YELLOW << "pipe_in created" << RESET);

	if (pipe(pipe_out) == -1) {
		close(pipe_in[0]);
		close(pipe_in[1]);
		DEBUG_LOG(RED << "Error: " << RESET << "handleCGI: pipe() failed for pipe_out");
		return (-1);
	}
	DEBUG_LOG(YELLOW << "pipe_out created" << RESET);

	_pid = fork();
	DEBUG_LOG(YELLOW << "fork() returned: " << _pid << RESET);
	if (_pid == -1) {
		DEBUG_LOG(RED << "Error: " << RESET << "handleCGI: fork() failed");
		closePipes(pipe_in, pipe_out);
		return (-1);
	}

	if (_pid == 0) { // child 
		DEBUG_LOG(YELLOW << "In child process" << RESET);
		dup2(pipe_in[0], STDIN_FILENO); // read end
		dup2(pipe_out[1], STDOUT_FILENO); // write end
		closePipes(pipe_in, pipe_out);

		char resolved_path[PATH_MAX];
		if (realpath(_scriptPath.c_str(), resolved_path) == NULL) {
			DEBUG_LOG(RED << "EXIT: " << RESET << "handleCGI: cannot get realpath");
			exit(EXIT_FAILURE);
		}

		char* argv[] = {
			const_cast<char*>(_route->cgiPath.c_str()),
			resolved_path,
			NULL
		};
		DEBUG_LOG(YELLOW << "Child calling execve with argv[0]=" << argv[0] << ", argv[1]=" << argv[1] << RESET);
		execve(argv[0], argv, _envv);

		std::cerr << RED << "EXECVE FAILED" << RESET << std::endl;

		DEBUG_LOG(RED << "EXIT: " << RESET << "handleCGI: execve() failed: " << strerror(errno) << " for absolute script path: " << resolved_path);
		exit(EXIT_FAILURE);
	} else { // parent 
		DEBUG_LOG(YELLOW << "In parent process, child pid=" << _pid << RESET);
		close(pipe_in[0]);
		close(pipe_out[1]);

		const std::vector<char>& body = _req.getBody();
		if (!body.empty()) {
			ssize_t written = write(pipe_in[1], &body[0], body.size());
			if (written < 0) {
				DEBUG_LOG(YELLOW << "Warning: write to CGI stdin failed: " << strerror(errno) << RESET);
			}
		}
		close(pipe_in[1]);
		
		_pipe_out_fd = pipe_out[0];
		DEBUG_LOG(YELLOW << "Parent returning pid=" << _pid << RESET);
		return (_pid);
	}
	return (-1);
}

pid_t	CGI::getPid() const { return (this->_pid); }

int	CGI::getPipeReadFd() const { return (this->_pipe_out_fd); }
