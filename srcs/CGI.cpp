#include "../includes/includes.hpp"

void	setEnvv(char ***envv, Request& req, Server& server, Route* route, const std::string& scriptPath) {
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

	std::string scriptBaseName;
	size_t lastSlash = scriptPath.rfind('/');
	if (lastSlash != std::string::npos)
		scriptBaseName = scriptPath.substr(lastSlash + 1);
	else 
		scriptBaseName = scriptPath;
	std::string pathInfo;
	std::string scriptName;
	size_t scriptPos = reqURL.find(scriptBaseName);
	if (scriptPos != std::string::npos) {
		scriptPos += scriptPath.length();
		scriptName = reqURL.substr(0, scriptPos);

		size_t queryPos = reqURL.find('?', scriptPos);
		if (queryPos != std::string::npos)
			pathInfo = reqURL.substr(scriptPos, queryPos - scriptPos);
		else 
			pathInfo = reqURL.substr(scriptPos);
	}
	std::vector<std::string> vect = getSNandPI(scriptPath, reqURL);
	envVector.push_back("SCRIPT_NAME=" + vect[0]);
	envVector.push_back("PATH_INFO=" + vect[1]);

	size_t pos = reqURL.find('?');
	std::string substr = "";
	if (pos != std::string::npos)
		substr = reqURL.substr(pos + 1);
	envVector.push_back("QUERY_STRING=" + substr);

	std::map<std::string, std::string> headersMap = req.getHeaders();
	std::map<std::string, std::string>::const_iterator it;
	for (it = headersMap.begin(); it != headersMap.end(); ++it) {
		std::string headerEnvv = "";
		std::string key = it->first;
		for (size_t i = 0; i < key.length(); ++i)
			key[i] = std::toupper(key[i]);
		std::replace(key.begin(), key.end(), '-', '_');
		if (key == "CONTENT_TYPE")
			envVector.push_back(key + "=" + it->second);
		else 
			envVector.push_back("HTTP_" + key + "=" + it->second);
	}

	*envv = new char*[envVector.size() + 1]; // FREEEEEEEE
	size_t i = 0;
	for (; i < envVector.size(); ++i)
		(*envv)[i] = strdup(envVector[i].c_str());
	(*envv)[i] = NULL;
}

void	handleCGI(const std::string& filename, Request& req, Server& server, Route* route) {
	char **envv = NULL;
	setEnvv(&envv, req, server, route, filename);

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
