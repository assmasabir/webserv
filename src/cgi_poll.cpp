#include "../includes/server.hpp"
#include "../includes/Cgi.hpp"
#include "../includes/HttpRequest.hpp"

void Server::cleanup_client(int client_fd)
{
    // Clean up CGI pipes for this client
    std::map<int, int>::iterator pipe_it = cgi_pipe_to_client.begin();
    while (pipe_it != cgi_pipe_to_client.end()) {
        if (pipe_it->second == client_fd) {
            int pipe_fd = pipe_it->first;
            close(pipe_fd);
            
            std::map<int, int>::iterator to_erase = pipe_it;
            ++pipe_it;
            cgi_pipe_to_client.erase(to_erase);
            
            // Remove from fds
            for (size_t j = 0; j < fds.size(); ++j) {
                if (fds[j].fd == pipe_fd) {
                    fds.erase(fds.begin() + j);
                    break;
                }
            }
        } else {
            ++pipe_it;
        }
    }
    
    // Clean up CGI handler
    std::map<int, CgiHandler*>::iterator cgi_it = active_cgi_handlers.find(client_fd);
    if (cgi_it != active_cgi_handlers.end()) {
        delete cgi_it->second;
        active_cgi_handlers.erase(cgi_it);
    }
    
    // Clean up request
    std::map<int, HttpRequest*>::iterator req_it = client_request.find(client_fd);
    if (req_it != client_request.end()) {
        delete req_it->second;
        client_request.erase(req_it);
    }
    
    client_status.erase(client_fd);
}

void Server::checkCgiTimeouts()
{
    std::map<int, CgiHandler*>::iterator it = active_cgi_handlers.begin();
    while (it != active_cgi_handlers.end()) {
        CgiHandler* cgi = it->second;
        int client_fd = it->first;
        
        time_t current_time = time(NULL);
        if (current_time - cgi->getCgiStartTime() > CGI_TIMEOUT_SECONDS)
        {
            std::cerr << "[CGI] Timeout - killing process " << cgi->getCgiPid() << std::endl;
            if (cgi->getCgiPid() > 0) {
			kill(cgi->getCgiPid(), SIGKILL);
            }
            cgi->setCgiError();
            cgi->setCgiStatus(504);

            for (size_t j = 0; j < fds.size(); ++j) {
                if (fds[j].fd == client_fd) {
                    fds[j].events = POLLOUT;
                    break;
                }
            }
        }        
        ++it;
    }
}

bool HasValidExtension(const std::string& path) {
    size_t pos = path.find_last_of('.');
    if (pos == std::string::npos) return false;
    
    std::string ext = path.substr(pos);
    return ext == ".py" || ext == ".php" || ext == ".sh";
}

void Server::AddCgiToPOLL(int client_fd){

    if (HasValidExtension(client_request[client_fd]->getPath()))
	{
		CgiHandler* cgi = new CgiHandler(*(client_request[client_fd]));
		active_cgi_handlers[client_fd] = cgi;

        cgi->setClientFd(client_fd);
        cgi->setDataClient(client_dataFile[client_fd]);
		
		while (!cgi->isCgiDone() && !cgi->isCgiError() && cgi->getCgiPipeFd() == -1) {
			cgi->handleCGIRequest();
		}
		
		if (cgi->isCgiError()) {
			fds[index].events = POLLOUT;
		}
		else {
			int pipe_fd = cgi->getCgiPipeFd();
			if (pipe_fd != -1) {
				// Add the pipe to poll monitoring
				struct pollfd cgi_pfd;
				cgi_pfd.fd = pipe_fd;
				cgi_pfd.events = POLLIN;
				cgi_pfd.revents = 0;
				fds.push_back(cgi_pfd);
				cgi_pipe_to_client[pipe_fd] = client_fd;
			}
		}
	}
	else
	{
		fds[index].events = POLLOUT;
	}
};