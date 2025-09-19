#include "../includes/includes.hpp"

CGI::CGI(Request &req, Server& server, const std::string& scriptPath, Response& res) : _req(req), _server(server), _scriptPath(scriptPath), _res(res) {
	_reqURL = req.getContent();
	_envv = NULL;
	_setEnvv();
}

CGI::~CGI() {
	if (_pid > 0) {
		pid_t child_pid;
		int status = 0;
		time_t startTime = time(NULL);

		while (true) {
			child_pid = waitpid(_pid, &status, WNOHANG);
			if (child_pid == _pid)
				break;
			if (time(NULL) - startTime > TIMEOUT_SECONDS) {
				kill(_pid, SIGKILL);
				_res.buildErrorResponse(504, _req, _server); // 504 Gateway timeout
				close(_pipe_out_fd);
				DEBUG_LOG(RED << "Error: " << RESET << "CGI timeout");
				return;
			}
			usleep(10000);
		}
		
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			_res.buildErrorResponse(500, _req, _server); // 500 Internal Server Error
			return;
		}
		
		_readCGI(_pipe_out_fd);	
		_parse();
	}

	if (_envv != NULL) {
		for (size_t i =0; _envv[i] != NULL; ++i)
			free(_envv[i]);
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
	envVector.push_back("CONTENT_LENGTH=" + toString(_req.getBody().size()));
	envVector.push_back("SERVER_NAME=" + _server.host);
	envVector.push_back("SERVER_PORT=" + toString(_server.port));
	envVector.push_back("REMOTE_ADDR=" + _req.getClientIP());

	_setQueryString();
	envVector.push_back("QUERY_STRING=" + _queryString);

	_setSNandPI();
	envVector.push_back("SCRIPT_NAME=" + _scriptName);
	envVector.push_back("PATH_INFO=" + _pathInfo);

	_setHeaders(envVector);

	_vectToArray(envVector);
}

void	CGI::_parse() {
	std::string	headers;

	size_t pos = _CGIoutput.find("\r\n\r\n");
	if (pos != std::string::npos) {
		headers = _CGIoutput.substr(0, pos);
		_parsedBody = _CGIoutput.substr(pos + 4); // +4 to skip \r\n\r\n
	} else 
		_parsedBody = _CGIoutput;
	
	std::istringstream iss(headers);
	std::string line;
	while (std::getline(iss, line)) {
		size_t pos = line.find(':');
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			value.erase(0, value.find_first_not_of(" \t")); // trim leading whitespace
			if (!value.empty() && value[value.size() - 1] == '\r')
				value.erase(value.size() - 1);
			_headersMap.insert(std::make_pair(key, value));
		}
	}

	_res.buildCGIResponse(*this);
}

// The goal of tis function is to create a child process that will transform into the CGI script
void	CGI::execute(Route* route) {
	int pipe_in[2]; // sending data to the script
	int pipe_out[2]; // getting data from the script 

	if (pipe(pipe_in) == -1) {
		_res.buildErrorResponse(500, _req, _server);
		DEBUG_LOG(RED << "Error: " << RESET << "handleCGI: pipe() failed for pipe_in");
		return;
	}

	if (pipe(pipe_out) == -1) {
		_res.buildErrorResponse(500, _req, _server);
		close(pipe_in[0]);
		close(pipe_in[1]);
		DEBUG_LOG(RED << "Error: " << RESET << "handleCGI: pipe() failed for pipe_out");
		return;
	}

	pid_t pid = fork();
	if (pid == -1) {
		DEBUG_LOG(RED << "Error: " << RESET << "handleCGI: fork() failed");
		closePipes(pipe_in, pipe_out);
		_res.buildErrorResponse(500, _req, _server);
		return;
	}

	if (pid == 0) { // child 
		dup2(pipe_in[0], STDIN_FILENO); // read end
		dup2(pipe_out[1], STDOUT_FILENO); // write end
		closePipes(pipe_in, pipe_out);
		char* argv[] = {
			const_cast<char*>(route->cgiPath.c_str()),
			const_cast<char*>(_scriptName.c_str()),
			NULL
		};
		execve(argv[0], argv, _envv);
		DEBUG_LOG(RED << "EXIT: " << RESET << "handleCGI: execve() failed");
		exit(EXIT_FAILURE);
	} else { // parent 
		close(pipe_in[0]);
		close(pipe_out[1]);
		if (!_req.getBody().empty())
			write(pipe_in[1], _req.getBody().c_str(), _req.getBody().length());
		close(pipe_in[1]);
		
		_pid = pid;
		_pipe_out_fd = pipe_out[0];
		}

	// 	pid_t child_pid;
	// 	int status = 0;
	// 	time_t startTime = time(NULL);

	// 	while (true) {
	// 		child_pid = waitpid(pid, &status, WNOHANG);
	// 		if (child_pid == pid)
	// 			break;
	// 		if (time(NULL) - startTime > TIMEOUT_SECONDS) {
	// 			kill(pid, SIGKILL);
	// 			_res.buildErrorResponse(504, _req, _server); // 504 Gateway timeout
	// 			close(pipe_out[0]);
	// 			DEBUG_LOG(RED << "Error: " << RESET << "CGI timeout");
	// 			return;
	// 		}
	// 		usleep(10000);
	// 	}

	// 	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
	// 		_res.buildErrorResponse(500, _req, _server); // 500 Internal Server Error
	// 		return;
	// 	}

	// 	_readCGI(pipe_out[0]);	
	// 	_parse();
	// }
}

void	handleCGI(Response& res, const std::string& filename, Request& req, Server& server, Route* route) {
	CGI	CGIobj(req, server, filename, res);
	CGIobj.execute(route);
}

// std::string CGI(const std::string& filePath,const std::string& body ){ // fct de quentin
// 	int pipefd[2];
// 	if (pipe(pipefd) == -1)
// 		throw std::runtime_error("Pipe failed");
// 	pid_t pid = fork();
// 	if (pid == -1)
// 		throw std::runtime_error("Fork failed");

// 	if (pid == 0) // child
// 	{
// 		// STDOUT → pipe[1]
// 		dup2(pipefd[1], STDOUT_FILENO);
// 		close(pipefd[0]); // close unused read

// 		if (!body.empty())
// 		{
// 			int inputPipe[2];
// 			pipe(inputPipe);
// 			write(inputPipe[1], body.c_str(), body.length());
// 			close(inputPipe[1]);
// 			dup2(inputPipe[0], STDIN_FILENO);
// 		}
// 		char* argv[] = { const_cast<char*>(filePath.c_str()), NULL };
// 		extern char **environ;
// 		execve(filePath.c_str(), argv, environ);
// 		exit(1); // si exec échoue
// 	}
// 	close(pipefd[1]); 
// 	char buffer[4096];
// 	std::string result;

// 	ssize_t bytesRead;
// 	while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
// 		result.append(buffer, bytesRead);
// 	close(pipefd[0]);

// 	waitpid(pid, NULL, 0);

// 	return result;
// }