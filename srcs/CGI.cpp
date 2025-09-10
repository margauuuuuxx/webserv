#include "../includes/includes.hpp"

void	setEnvv(char ***envv, Request& req, Server& server, const std::string& scriptPath) {
	std::vector<std::string> envVector;
	std::string reqURL = req.getContent();

	envVector.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envVector.push_back("SERVER_PROTOCOL=" + req.getVersion());
	envVector.push_back("REQUEST_METHOD=" + req.getMethod());
	envVector.push_back("SCRIPT_FILENAME=" + scriptPath);
	envVector.push_back("CONTENT_LENGTH=" + std::to_string(req.getBody().size()));
	envVector.push_back("SERVER_NAME=" + server.host);
	envVector.push_back("SERVER_PORT=" + std::to_string(server.port));
	envVector.push_back("REMOTE_ADDR=" + req.getClientIP()); 
	envVector.push_back("QUERY_STRING=" + getQueryString(reqURL));

	std::vector<std::string> vect = getSNandPI(scriptPath, reqURL);
	envVector.push_back("SCRIPT_NAME=" + vect[0]);
	envVector.push_back("PATH_INFO=" + vect[1]);

	getHeaders(envVector, req);

	*envv = vectToArray(envVector);
}

void	handleCGI(const std::string& filename, Request& req, Server& server, Route* route) {
	char **envv = NULL;
	setEnvv(&envv, req, server, filename);

	(void)filename;
	(void)req;
	(void)server;
	(void)route;
	// implement Margaux's version

	// FREE ENVV AT THE END
}

std::string CGI(const std::string& filePath,const std::string& body ){ // fct de quentin
	int pipefd[2];
	if (pipe(pipefd) == -1)
		throw std::runtime_error("Pipe failed");
	pid_t pid = fork();
	if (pid == -1)
		throw std::runtime_error("Fork failed");

	if (pid == 0) // child
	{
		// STDOUT → pipe[1]
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[0]); // close unused read

		if (!body.empty())
		{
			int inputPipe[2];
			pipe(inputPipe);
			write(inputPipe[1], body.c_str(), body.length());
			close(inputPipe[1]);
			dup2(inputPipe[0], STDIN_FILENO);
		}
		char* argv[] = { const_cast<char*>(filePath.c_str()), NULL };
		extern char **environ;
		execve(filePath.c_str(), argv, environ);
		exit(1); // si exec échoue
	}
	close(pipefd[1]); 
	char buffer[4096];
	std::string result;

	ssize_t bytesRead;
	while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
		result.append(buffer, bytesRead);
	close(pipefd[0]);

	waitpid(pid, NULL, 0);

	return result;
}
