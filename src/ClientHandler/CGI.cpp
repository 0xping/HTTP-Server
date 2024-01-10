#include "ClientHandler.hpp"

void ClientHandler::execCGI(){
    char nameBuffer[L_tmpnam] = {0};
	std::string cgioutput = std::tmpnam(nameBuffer);
    tmpFiles.push_back(cgioutput);
	
	pid_t pid = fork(); 
	if (pid){
		if (pid < 0)
			setResponseParams("500", "Internal Server Error", "", "");
		int status;
		clock_t startTime = clock();
		while (waitpid(pid, &status, WNOHANG) == 0){
			clock_t endTime = clock();
			double elapsedTime = static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;
			if (elapsedTime >= 30){
				setResponseParams("500", "Internal Server Error", "", "");
				kill(pid, SIGTERM);
				return ;
			}
		}
    	if (WIFEXITED(status)){
			if (!WEXITSTATUS(status))
				setResponseParams("200", "OK", "", cgioutput, true);
			else
				setResponseParams("500", "Internal Server Error", "", "");
		}
		else if (WIFSIGNALED(status))
			setResponseParams("500", "Internal Server Error", "", "");
    	
	}
	else{
		int fd = open(cgioutput.c_str(), O_WRONLY | O_CREAT);
		if (fd < 0)
			exit(1);
		const char *args[] = {CGIpath.c_str(), fullLocation.c_str(), postedFileName.c_str(), NULL};
		const char *env[] = {query.c_str(), NULL};
		if (dup2(fd,1) == -1)
			exit(1);
		execve(fullLocation.c_str(), (char *const *)args, (char *const *)env);
		exit(1);
	}
}