#include "../includes/webserv.hpp"

int Socket :: create_listening_socket(std::string &ip, int port)
{
	int	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
		throw (SocketFailedException());
	int	opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) < 0)
	{
		close(server_fd);
		throw (SetsockoptFailedException());
	}
	struct sockaddr_in	address;
	socklen_t addrlen = sizeof address;
	bzero(&address, addrlen);
	address.sin_family = AF_INET;
	if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) != 1)
	{
		close(server_fd);
		throw (InetFailedException());
	}
	address.sin_port = htons(port);
	if (bind(server_fd, (struct sockaddr *)&address, addrlen) < 0)
	{
		close(server_fd);
		throw (BindFailedException());
	}
	if (listen(server_fd, 100) < 0) // sets the server to passive mode
	{
		close(server_fd);
		throw (ListenFailedException());
	}
	int	flags = fcntl(server_fd, F_GETFL, 0);
	if (flags < 0 || fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		close(server_fd);
		throw (ServerSocketNonBlockException());
	}
	return (server_fd);
}

Socket :: Socket(std::list<struct ServerConfig> &serverList)
{
	for (std::list<ServerConfig>::iterator it = serverList.begin(); it != serverList.end(); ++it)
	{
		std::vector<std::pair<int,std::string> >::iterator lp = it->interface_port.begin();
		for ( ; lp != it->interface_port.end(); ++lp) {
			int	port = lp->first;
			std::string ip = lp->second;
			std::pair<int, std::string>	key(port, ip);
			if (listener_fds.find(key) == listener_fds.end())
			{
				int	fd = create_listening_socket(ip, port);
				listener_fds[key] = fd;
			}
		}
	}
}

Socket :: ~Socket()
{
}

std::map< std::pair<int, std::string>, int > Socket :: get_listener_fds() const
{
	return (listener_fds);
}

const char  *Socket::SocketFailedException::what() const throw()
{
	return ("Error: socket() failed");
}

const char  *Socket::SetsockoptFailedException::what() const throw()
{
	return ("Error: setsockopt() failed");
}

const char  *Socket::InetFailedException::what() const throw()
{
	return ("Error: inet_pton() failed");
}

const char  *Socket::BindFailedException::what() const throw()
{
	return ("Error: bind() failed");
}

const char  *Socket::ListenFailedException::what() const throw()
{
	return ("Error: listen() failed");
}

const char  *Socket::ServerSocketNonBlockException::what() const throw()
{
	return ("Error: failed to make server socket non-blocking");
}
