#include "../includes/webserv.hpp"

bool	server_parser(std::vector<std::string> &tokens,
		std::list<struct ServerConfig> &serverList)
{
	if (tokens.size() != 2 || tokens[1] != "{")
		return true;
	if (!serverList.empty() && serverList.back().section_closed == false)
			return true;
	struct ServerConfig	tmp;
	tmp.section_closed = false;
  tmp.max_body_size = 0;
	serverList.push_back(tmp);
	return false;
}

bool	rightCurlyBrace(std::vector<std::string> &tokens,
		std::list<struct ServerConfig> &serverList)
{
	if (tokens.size() != 1)
		return true;
	if (serverList.empty() || serverList.back().section_closed == true)
		return true;
	////////////////////////
	if (!serverList.back().locations.empty())
	{
		if (serverList.back().locations.back().section_closed == false)
		{
			if (serverList.back().locations.back().root.empty())
			{
				std::cout << "Error: Expected root "
					<< "directive inside location section\n";
				return (-1);
			}
			serverList.back().locations.back().section_closed = true;
			if (serverList.back().locations.back().allowed_methods.empty()
					&& serverList.back().locations.back().no_allowed_methods == false)
			{
				serverList.back().locations.back().allowed_methods.push_back("GET");
				serverList.back().locations.back().allowed_methods.push_back("POST");
				serverList.back().locations.back().allowed_methods.push_back("DELETE");
			}
		}
		else
		{
			if (serverList.back().interface_port.empty())
			{
				std::cout << "Error: Expected "
					<< "interface port inside server section\n";
				return true;
			}
			if (serverList.back().locations.empty())
			{
				std::cout << "Error: Expected "
					<< "location block inside server section\n";
				return true;
			}
			if (serverList.back().default_type.empty())
			{
				std::cout << "Error: Expected "
					<< "default type directive inside server section\n";
				return true;
			}
			serverList.back().section_closed = true;
		}
	}
	else if (serverList.back().section_closed == false)
	{
		if (serverList.back().interface_port.empty())
		{
			std::cout << "Error: Expected "
				<< "interface port directive inside server section\n";
			return true;
		}
		if (serverList.back().locations.empty())
		{
			std::cout << "Error: Expected "
				<< "location block inside server section\n";
			return true;
		}
		if (serverList.back().default_type.empty())
		{
			std::cout << "Error: Expected "
				<< "default type directive inside server section\n";
			return true;
		}
		serverList.back().section_closed = true;
	}
	else
		return true;
	return false;
}
