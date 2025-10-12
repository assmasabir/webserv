#include "../includes/webserv.hpp"

bool	directory_listing_parser(std::vector<std::string> &tokens,
		std::list<struct ServerConfig> &serverList)
{
	if (tokens.size() != 2)
		return true;
	if (serverList.empty() || serverList.back().locations.empty()
		|| serverList.back().locations.back().section_closed == true)
		return true;
	size_t	pos = tokens[1].find(";");
	if (pos != tokens[1].size() - 1)
		return true;
	tokens[1].resize(tokens[1].size() - 1);
	if (tokens[1] == "on")
		serverList.back().locations.back().directory_listing = true;
	else if (tokens[1] == "off")
		serverList.back().locations.back().directory_listing = false;
	else
		return true;
	return false;
}
