#include "../includes/webserv.hpp"

bool	location_parser(std::vector<std::string> &tokens,
		std::list<struct ServerConfig> &serverList)
{
	if (tokens.size() != 3 || tokens[2] != "{")
		return true;
	if (serverList.empty()
		|| (!serverList.back().locations.empty() && serverList.back().locations.back().section_closed == false)
		|| serverList.back().section_closed == true)
		return true;
	struct LocationConfig	tmp;
	tmp.section_closed = false;
	tmp.directory_listing = false;
	tmp.no_allowed_methods = false;
	tmp.path = tokens[1];
	serverList.back().locations.push_back(tmp);
	return false;
}
