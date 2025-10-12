#ifndef SOCKET_HPP
# define SOCKET_HPP

#include "webserv.hpp"

class Socket
{
	private:
    std::map< std::pair<int, std::string>, int > listener_fds;
	public:
		Socket(std::list<struct ServerConfig> &);
    	int create_listening_socket(std::string &ip, int port);
		~Socket();
    	std::map< std::pair<int, std::string>, int > get_listener_fds() const;
		class SocketFailedException : public std::exception
		{
			public:
				const char  *what() const throw();
		};
		class SetsockoptFailedException : public std::exception
		{
			public:
				const char  *what() const throw();
		};
		class InetFailedException : public std::exception
		{
			public:
				const char  *what() const throw();
		};
		class BindFailedException : public std::exception
		{
			public:
				const char  *what() const throw();
		};
		class ListenFailedException : public std::exception
		{
			public:
				const char  *what() const throw();
		};
		class ServerSocketNonBlockException : public std::exception
		{
			public:
				const char  *what() const throw();
		};
};

#endif
