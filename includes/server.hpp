#ifndef SERVER_HPP
# define SERVER_HPP

#include "webserv.hpp"
#include "HttpRequest.hpp"

class Cgihandler;

class	Server
{
	private:
		std::vector<struct pollfd>	fds;
		size_t             		 			index;
		std::map<int, std::fstream *>		client_dataFile;
		std::map<int, time_t>				client_time;
		std::map<int, std::string>			client_fileName;
		std::map<int, int>					client_flag;
		std::map<int, int> 					cgi_pipe_to_client;    // Pipe FD -> Client FD
		std::map<int, CgiHandler*> 			active_cgi_handlers;   // Client FD -> CGI handler
		std::map<int, HttpRequest*> 		client_request;       // Client FD -> Request object
		std::map<int, int> 					client_status;         // Client FD -> Status code
	public:
		Server(const std::map< std::pair<int, std::string>, int > &);
		~Server();
		void  run(std::list<struct ServerConfig> &serverList);
		void  _accept();
		void	createFileForRequest(int client_fd, std::string &fileName);
		Server();
		void  handle_client(std::list<struct ServerConfig> &serverList);
		void	oldClientSendingBody(int client_fd, std::string &buffer);
		void	disconnectClient(int client_fd);
		void	TransferEncodingParser(int client_fd, std::string &body);
		void	ContentLengthParser(int client_fd, std::string &body);
		int	SavePostDataInUploads(int client_fd);
		void	removeTimedOutClients();
		void	removeOneClient(int fd);
		void  _respond();

		void AddCgiToPOLL(int client_fd);
		void checkCgiTimeouts();
		void cleanup_client(int client_fd);
		void debugPrintState();
		void ft_free();

		class PollFailedException : public std::exception
		{
			public:
			const char  *what() const throw();
		};
};

#endif
