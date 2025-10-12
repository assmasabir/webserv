#include "../includes/webserv.hpp"

void	display_allowed_methods(std::vector<std::string> &allowed_methods)
{
	std::cout << "allowed_methods : ";
	for (size_t i = 0 ; i < allowed_methods.size() ; i++)
		std::cout << allowed_methods[i] << " ";
	std::cout << "\n";
}

void	display_cgi(std::map<std::string, std::string> &cgi)
{
	std::map<std::string, std::string>::iterator it;

	for (it = cgi.begin() ; it != cgi.end() ; it++)
	{
		std::cout << "cgi extension : " << it->first << "\n";
		std::cout << "cgi location : " << it->second << "\n";
	}
}

void	display_locations_infos(std::vector<struct LocationConfig> locations)
{
	for (size_t i = 0 ; i < locations.size() ; i++)
	{
		std::cout << "location : \n";
		std::cout << "path : " << locations[i].path << "\n";
		std::cout << "root : " << locations[i].root << "\n";
		display_allowed_methods(locations[i].allowed_methods);
		display_cgi(locations[i].cgi);
		std::cout << "upload directory : " << locations[i].upload_directory << "\n";
		std::cout << "default file : " << locations[i].default_file << "\n";
		std::cout << "directory listing : " << locations[i].directory_listing << "\n";
		std::cout << "redirect : " << locations[i].redirect << "\n";
	}
}

void	display_error_infos(std::map<int, std::string> &error)
{
	std::map<int, std::string>::iterator it;

	for (it = error.begin() ; it != error.end() ; it++)
	{
		std::cout << "error code : " << it->first << "\n";
		std::cout << "error uri : " << it->second << "\n";
	}
}

void	display_extension_path(std::map<std::string, std::string> &m)
{
	std::map<std::string, std::string>::iterator it;

	for (it = m.begin() ; it != m.end() ; it++)
	{
		std::cout << "extension : " << it->first << "\n";
		std::cout << "path : " << it->second << "\n";
	}
}

void	display_interface_port(std::vector<std::pair<int, std::string> > &interface_port)
{
	std::vector<std::pair<int, std::string> >::iterator it;

	for (it = interface_port.begin() ; it != interface_port.end() ; it++)
	{
		std::cout << "port to listen on : " << it->first << "\n";
		std::cout << "host : " << it->second << "\n";
	}
}

void	display_config_content(std::list<struct ServerConfig> &serverList)
{
	std::list<struct ServerConfig>::iterator	it;

	for (it = serverList.begin() ; it != serverList.end() ; it++)
	{
		std::cout << "server : \n";
		// display_interface_port(it->interface_port);
		// display_extension_path(it->extension_path);
		// std::cout << "max body size : " << it->max_body_size << "\n";
		// display_error_infos(it->error);
		//display_locations_infos(it->locations);
	}
}
