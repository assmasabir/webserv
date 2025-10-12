#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <ctime>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/stat.h>
#include <algorithm>
#include <climits>
#include <sys/types.h>
 #include <dirent.h>
#include "socket.hpp"
// #include "server.hpp"

typedef struct LocationConfig {
	std::string 						path;
	std::string 						root;
	std::vector<std::string> 			allowed_methods;
	bool								no_allowed_methods;
	std::map<std::string, std::string>	cgi;
	std::string							upload_directory;
	std::string							default_file;
	bool								directory_listing;
	std::string							redirect;
	bool								section_closed;
} locationconfig;

typedef struct ServerConfig {
	std::vector<std::pair<int, std::string> >	interface_port;
	std::map<int, std::string>					error;
	size_t										max_body_size;
	std::vector<LocationConfig> 				locations;
	std::string									content_type;
	std::string									default_type;
	std::map<std::string, std::string>			extension_path;
	bool	section_closed;
} serverconfig;

extern volatile sig_atomic_t  g_running;

int	parse_config(char *, std::list<struct ServerConfig> &);
bool	is_regular_file(std::string &);
void	splitLine(std::string &, std::vector<std::string> &);
bool	server_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	rightCurlyBrace(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	listen_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	location_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	interface_port_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	root_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	allowed_methods_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	directory_listing_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	default_type_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	default_file_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	upload_directory_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	redirect_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	max_body_size_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	error_page_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	cgi_parser(std::vector<std::string> &,
		std::list<struct ServerConfig> &);
bool	content_type_parser(std::vector<std::string> &tokens,
		std::list<struct ServerConfig> &serverList);
void	display_config_content(std::list<struct ServerConfig> &);

#endif
