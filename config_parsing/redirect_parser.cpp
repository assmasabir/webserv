#include "../includes/webserv.hpp"

bool	redirect_parser(std::vector<std::string> &tokens,
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
	serverList.back().locations.back().redirect = tokens[1];
	return false;
}
