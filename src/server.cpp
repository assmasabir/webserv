#include "../includes/server.hpp"
#include "../includes/Cgi.hpp"
#include "../includes/HttpRequest.hpp"


Server :: Server(const std::map< std::pair<int, std::string>, int > &listener_fds)
{
	std::map<std::pair<int, std::string>, int>::const_iterator it;
	for (it = listener_fds.begin() ; it != listener_fds.end() ; ++it)
	{
		struct pollfd pfd;
		pfd.fd = it->second;
		pfd.events = POLLIN;
		pfd.revents = 0;
		fds.push_back(pfd);
	}
}

void	Server :: _accept()
{
	int	client_fd;
	while ((client_fd = accept(fds[index].fd,
		(struct sockaddr *)NULL, NULL)) >= 0)
	{
		int	client_flags = fcntl(client_fd, F_GETFL, 0);
		if (client_flags != 1)
			fcntl(client_fd, F_SETFL, client_flags | O_NONBLOCK);
		fds.push_back((struct pollfd){client_fd, POLLIN, 0});
		time_t	timer = time(NULL);
		client_time[client_fd] = timer;
		char	*time_msg = ctime(&timer);
		// std::cout << "New client: " << client_fd << "\n";
		std::cout << client_fd << " joined at: " << time_msg << "\n";
	}
	if (errno != EAGAIN && errno != EWOULDBLOCK)
		std::cerr << "accept failed\n";
}

bool	is_listening_fd(int fd, std::set<int> &listening_fds)
{
	return (listening_fds.count(fd) > 0);
}

std::string	generate_fileName(int fd)
{
	(void)fd;
	std::stringstream	ss;
	std::srand(time(NULL));
	ss << std::rand();
	std::string	fileName = "/tmp/";
	fileName += ss.str();
	fileName += "_webserv.tmp";
	return (fileName);
}

void	Server :: createFileForRequest(int client_fd, std::string &fileName)
{
	client_dataFile[client_fd] = new std::fstream(fileName.c_str(),
		std::ios::in | std::ios::out | std::ios::binary);
	if (!client_dataFile[client_fd]->is_open())
	{
		std::ofstream create(fileName.c_str(), std::ios::binary);
		create.close();
		client_dataFile[client_fd]->open(fileName.c_str(),
			std::ios::in | std::ios::out | std::ios::binary);
	}
}

std::fstream	*createFile(std::string &fileName)
{
	std::fstream	*file = new std::fstream(fileName.c_str(),
		std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
	if (!file->is_open())
	{
		std::ofstream create(fileName.c_str(), std::ios::binary);
		create.close();
		file->open(fileName.c_str(),
			std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
		if (!file->is_open())
		{
			std::cout << "Couldn't open the file to store the body\n";
			return NULL;
		}
	}
	return (file);
}

int	Server :: SavePostDataInUploads(int client_fd)
{
	client_dataFile[client_fd]->seekg(0);
	std::string	content_type = client_request[client_fd]->getContentType();
	if (content_type == "NO_BOUNDARY")
	{
		std::string	fileName = "src/www/uploads/random";
		std::fstream	*file = createFile(fileName);
		if (!file)
			return (ERROR_500);
		std::string	line;
		while (getline(*client_dataFile[client_fd], line))
		{
			// std::cout << "line : " << line << "\n";
			*file << line << "\n";
		}
		file->flush();
		file->close();
		delete file;
		return (PARSE_OK);
	}
	std::string	line;
	std::string	boundry = client_request[client_fd]->getBoundary();
	while (getline(*client_dataFile[client_fd], line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line == boundry)
			break ;
	}
	if (line.empty())
		return (ERROR_400);
	while (getline(*client_dataFile[client_fd], line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line == boundry)
			break ;
		std::stringstream	ss(line);
		std::string	token;
		ss >> token;
		if (token != "Content-Disposition:")
			continue ;
		ss >> token;
		if (token != "form-data;")
			continue ;
		ss >> token;
		if (token != "name=\"file-upload\";" && token != "name=\"file\";")
			continue ;
		ss >> token;
		std::string	rest;
		std::getline(ss, rest, '\0');
		token += rest;
		if (token.compare(0, 10, "filename=\""))
			continue ;
		std::string	fileName = token.substr(10);
		if (fileName[fileName.size() - 1] != '"')
			continue ;
		fileName.erase(fileName.size() - 1);
		if (fileName.empty() || fileName.find("/") != std::string::npos
			|| fileName.find("..") != std::string::npos)
			return ERROR_422;
		fileName = "src/www/uploads/" + fileName;
		getline(*client_dataFile[client_fd], line);
		if (line.empty())
			return ERROR_400;
		if (!getline(*client_dataFile[client_fd], line))
			return ERROR_400;
		std::fstream	*file = createFile(fileName);
		if (!file)
			return (ERROR_500);
		boundry += "--";
		while (getline(*client_dataFile[client_fd], line))
		{
			if (line == "\r")
			{
				if (!getline(*client_dataFile[client_fd], line))
				{
					if (line.find(boundry) != std::string::npos)
					{
						file->flush();
						file->close();
						delete	file;
						return PARSE_OK;
					}
					*file << "\r\n" << line << "\n";
				}
				else
					break ;
			}
			else
			{
				if (line.find(boundry) != std::string::npos)
				{
					file->flush();
					file->close();
					delete	file;
					return PARSE_OK;
				}
				*file << line << "\n";
			}
		}
		file->flush();
		file->close();
		delete	file;
		if (line.empty())
		{
			// std::cout << "HERE\n";
			return (ERROR_400);
		}
	}
	return (PARSE_OK);
}

void	Server :: ContentLengthParser(int client_fd, std::string &body)
{
	if (client_request[client_fd]->getContentLength() <= 0)
	{
		client_status[client_fd] = SavePostDataInUploads(client_fd);
		client_flag.erase(client_fd);
		AddCgiToPOLL(client_fd);
		return ;
	}
	client_request[client_fd]->decrementContentLength(body.size());
	if (client_request[client_fd]->getContentLength() <= 0)
	{
		client_status[client_fd] = SavePostDataInUploads(client_fd);
		client_flag.erase(client_fd);
		AddCgiToPOLL(client_fd);
		return ;
	}
}

void	Server :: TransferEncodingParser(int client_fd, std::string &body)
{
	if (body.find("0\r\n\r\n") == std::string::npos)
		return ;
	client_dataFile[client_fd]->seekg(0);
	std::string	tmpFileName = "/tmp/tmp_webserv.tmp";
	std::fstream	*tmpFile = createFile(tmpFileName);
	if (!tmpFile)
	{
		client_status[client_fd] = ERROR_500;
		std::cout << "Error in chunked body\n";
		client_flag.erase(client_fd);
		fds[index].events = POLLOUT;
		return ;
	}
	int	status = client_request[client_fd]->parse_body(client_dataFile[client_fd], tmpFile);
	client_status[client_fd] = status;
	if (status != PARSE_OK)
	{
		std::cout << "Error in chunked body\n";
		client_flag.erase(client_fd);
		fds[index].events = POLLOUT;
		return ;
	};
	tmpFile->flush();
	tmpFile->seekg(0);
	client_dataFile[client_fd]->close();
	delete client_dataFile[client_fd];
	client_dataFile[client_fd] = tmpFile;
	client_status[client_fd] = SavePostDataInUploads(client_fd);
	client_flag.erase(client_fd);
	AddCgiToPOLL(client_fd);
}

void	Server :: oldClientSendingBody(int client_fd, std::string &buffer)
{
	*client_dataFile[client_fd] << buffer;
	client_dataFile[client_fd]->flush();
	if (client_request[client_fd]->getTransferEncodingFlag())
		TransferEncodingParser(client_fd, buffer);
	else
		ContentLengthParser(client_fd, buffer);
}

void	Server :: disconnectClient(int client_fd)
{
	std::cout << "Client disconnected: " << client_fd << "\n";
	close(client_fd);
	removeOneClient(client_fd);
	client_status.erase(client_fd);
	client_time.erase(client_fd);
	if (client_request.count(client_fd) > 0)
		delete client_request[client_fd];
	client_request.erase(client_fd);
	client_flag.erase(client_fd);
	index--;
}

void  Server :: handle_client(std::list<struct ServerConfig> &serverList)
{
	int	client_fd = fds[index].fd;
	std::string	buffer(4096, '0');
	int	bytes_received = read(client_fd, &buffer[0], 4096);
	if (bytes_received <= 0)
	{
		disconnectClient(client_fd);
		return ;
	}
	buffer.resize(bytes_received);
	if (client_time.count(client_fd) > 0)
		client_time[client_fd] = time(NULL);
	// std::cout << "buffer : " << buffer << "\n";
	client_fileName[client_fd] = generate_fileName(client_fd);
	if (client_dataFile.count(client_fd) <= 0)
		createFileForRequest(client_fd, client_fileName[client_fd]);
	if (client_flag.count(client_fd) > 0)
	{
		oldClientSendingBody(client_fd, buffer);
		return ;
	}
	size_t	pos = buffer.find("\r\n\r\n");
	if (pos == std::string::npos) // request headers not yet finished
	{
		*client_dataFile[client_fd] << buffer;
		return ;
	}
	else
	{
		// buff the first part, parse it and save the other to the body
		std::string	last_headers = buffer.substr(0, pos);
		*client_dataFile[client_fd] << last_headers << "\r\n\r\n";
		client_request[client_fd] = new HttpRequest();
		HttpRequest	*req = client_request[client_fd];
		int status = req->parse_request(client_dataFile[client_fd],
			serverList, client_fd);
		if (status != PARSE_OK)
		{
			client_status[client_fd] = status;
			fds[index].events = POLLOUT;
			return ;
		}
		client_dataFile[client_fd]->close();
		client_dataFile[client_fd]->open(client_fileName[client_fd].c_str(),
		std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
		if (!client_dataFile[client_fd]->is_open())
		{
			std::cout << "Couldn't open the file to store the request\n";
			client_status[client_fd] = ERROR_500;
			fds[index].events = POLLOUT;
			return ;
		}
		client_dataFile[client_fd]->seekp(0);
		client_status[client_fd] = status;
		if (client_request[client_fd]->getBodyFlag()) // POST method
		{
			client_flag.insert(std::make_pair(client_fd, 1));
			std::string	body = buffer.substr(pos + 4);
			// std::cout << "body : " << body << "\n";
			oldClientSendingBody(client_fd, body);
		}
		else
		{
			AddCgiToPOLL(client_fd);
			// fds[index].events = POLLOUT;
		}
	}
}

void  Server :: _respond()
{
	int	client_fd = fds[index].fd;
	std::string response;

	// Check if this client has an active CGI handler
    std::map<int, CgiHandler*>::iterator cgi_it = active_cgi_handlers.find(client_fd);
    if (cgi_it != active_cgi_handlers.end()) {
        CgiHandler* cgi = cgi_it->second;
        if (cgi->isCgiError()) {
            if (cgi->getCgiChildStatus() == 504) //time out
                response = client_request[client_fd]->handle_error_status(ERROR_504);
            else
                response = client_request[client_fd]->handle_error_status(ERROR_500);
        } 
        else if (cgi->isCgiDone()) {
            response = client_request[client_fd]->read_cgi_result(*cgi);
        } 
    }

    else {
        // std::cout << "[DEBUG] No CGI handler, using regular response" << std::endl;
        std::map<int, HttpRequest*>::iterator req_it = client_request.find(client_fd);
        if (req_it != client_request.end()) {
            response = req_it->second->prepare_response(client_status[client_fd]);
        } else {
            response = client_request[client_fd]->handle_error_status(ERROR_500);
        }
    }
	
	if (write(client_fd, response.c_str(), response.size()) <= 0)
	{
		cleanup_client(client_fd);
		disconnectClient(client_fd);
		return ;
	}
	cleanup_client(client_fd);
	close(client_fd);
	removeOneClient(client_fd);
	client_status.erase(client_fd);
	client_time.erase(client_fd);
	if (client_request.count(client_fd) > 0)
		delete client_request[client_fd];
	client_request.erase(client_fd);
	client_flag.erase(client_fd);
	--index;
}

void	Server :: removeOneClient(int fd)
{
	for (std::vector<struct pollfd>::iterator it2 = fds.begin()
			; it2 != fds.end() ; ++it2)
	{
		if (it2->fd == fd)
		{
			fds.erase(it2);
			break ;
		}
	}
	for (std::map<int, std::fstream *>::iterator it3 = client_dataFile.begin()
			; it3 != client_dataFile.end() ; ++it3)
	{
		if (it3->first == fd)
		{
			it3->second->close();
			delete it3->second;
			client_dataFile.erase(fd);
			break ;
		}
	}
	for (std::map<int, std::string>::iterator it4 = client_fileName.begin()
			; it4 != client_fileName.end() ; ++it4)
	{
		if (it4->first == fd)
		{
			// if (unlink(it4->second.c_str()) != 0)
			// {
			// 	std::cout << "Error: couldn't delete the file "
			// 						<< it4->second << "\n";
			// }
			client_fileName.erase(fd);
			break ;
		}
	}
	for (std::map<int, HttpRequest *>::iterator it5 = client_request.begin()
			; it5 != client_request.end() ; ++it5)
	{
		if (it5->first == fd)
		{
			delete it5->second;
			client_request.erase(fd);
			break ;
		}
	}
	for (std::map<int, CgiHandler *>::iterator it6 = active_cgi_handlers.begin()
			; it6 != active_cgi_handlers.end() ; ++it6)
	{
		if (it6->first == fd)
		{
			delete it6->second;
			active_cgi_handlers.erase(fd);
			break ;
		}
	}
}

void	Server :: removeTimedOutClients()
{
	time_t	current_time = time(NULL);
	for (std::map<int, time_t>::iterator it = client_time.begin()
			; it != client_time.end() ; ++it)
	{
		if (current_time - it->second > 12) // 3s for time out temporary
		{
			std::cout << it->first << " took way more time\n";
			close(it->first); // close connection
			removeOneClient(it->first);
			client_status.erase(it->first);
			client_flag.erase(it->first);
			cgi_pipe_to_client.erase(it->first);
			client_time.erase(it); // must stay HERE
			break ;
		}
	}
}

void Server::ft_free()
{
    for (std::map<int, CgiHandler*>::iterator it = active_cgi_handlers.begin(); 
        it != active_cgi_handlers.end(); ++it) {
        delete it->second;
    }
    active_cgi_handlers.clear();

    for (std::map<int, HttpRequest*>::iterator it = client_request.begin(); 
        it != client_request.end(); ++it) {
        delete it->second;
    }
    client_request.clear();

	for (std::map<int, std::fstream *>::iterator it = client_dataFile.begin(); 
        it != client_dataFile.end(); ++it) {
        delete it->second;
    }
    client_dataFile.clear();
}

void	Server :: run(std::list<struct ServerConfig> &serverList)
{
	std::set<int> listening_fds;

	for (size_t i = 0 ; i < fds.size() ; i++)
		listening_fds.insert(fds[i].fd);
	while (g_running)
	{
		if (poll(&fds[0], fds.size(), 1) < 0)
		{
			if (errno == EINTR)
				continue ;
			throw (PollFailedException());
		}
		if (!g_running)
		{
			break ;
		}
			
		removeTimedOutClients();
		checkCgiTimeouts();
		for (index = 0 ; index < fds.size() ; index++)
		{
			int flag = 0;
			for (std::map<int , int>::iterator it = cgi_pipe_to_client.begin(); it != cgi_pipe_to_client.end(); it++)
			{
				if (fds[index].fd == it->first || fds[index].fd == it->second)
				{
					flag = 1;
					break;
				}
			}
			if ((fds[index].revents & (POLLHUP | POLLIN)) && flag == 1)
			{
				// std::cout << "[DEBUG] CGI POLLIN" << std::endl;
				std::map<int, int>::iterator pipe_it = cgi_pipe_to_client.find(fds[index].fd);
				if (pipe_it != cgi_pipe_to_client.end())
                {   
                    int pipe_fd = fds[index].fd;
                    int client_fd = pipe_it->second;
                    
                    std::map<int, CgiHandler*>::iterator cgi_it = active_cgi_handlers.find(client_fd);
                    if (cgi_it != active_cgi_handlers.end())
                    {
                        CgiHandler* cgi = cgi_it->second;
                        cgi->handleCGIRequest();
                        
                        if (cgi->getCgiState() == CGI_STATE_WAIT_CHILD)
                        {
                            while (!cgi->isCgiDone() && !cgi->isCgiError() ) {
								cgi->handleCGIRequest();
						    }
                        }

                        // Check if CGI is done after reading
                        if (cgi->isCgiDone() || cgi->isCgiError()) {                            
                            // Clean up pipe from poll
                            close(pipe_fd);
                            cgi_pipe_to_client.erase(pipe_fd);
                            
                            // Remove pipe fd from poll array
                            for (size_t j = 0; j < fds.size(); ++j) {
                                if (fds[j].fd == pipe_fd) {
                                    fds.erase(fds.begin() + j);
                                    --index; 
                                    break;
                                }
                            }  
                            // Set client fd to POLLOUT to send response
                            for (size_t j = 0; j < fds.size(); ++j) {
                                if (fds[j].fd == client_fd) {
                                    fds[j].events = POLLOUT;
                                    break;
                                }
                            }
                        }
                    }
                }
			}
			
			else if (fds[index].revents & POLLIN)
			{
				if (is_listening_fd(fds[index].fd, listening_fds))
				{
                    // std::cout << "[DEBUG] ACCEPT" << std::endl;
                    _accept();
                }
				else
				{
                    // std::cout << "[DEBUG] HANDLE CLIENT" << std::endl;
                    handle_client(serverList);
                }
			}
			if (fds[index].revents & POLLOUT)
			{
                // std::cout << "[DEBUG] RESPOND" << std::endl;
                _respond();
            }
		}
	}
}

Server :: ~Server()
{
	ft_free();
	for (size_t i = 0 ; i < fds.size() ; i++)
	{
		close(fds[i].fd);
	}
}

const char  *Server::PollFailedException::what() const throw()
{
	return ("Error: poll() failed");
}
