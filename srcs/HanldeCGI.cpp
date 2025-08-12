#include "../includes.hpp"

std::string handleCgi(const std::string& filePath,const std::string& body ){
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
