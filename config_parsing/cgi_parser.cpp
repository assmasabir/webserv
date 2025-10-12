#include "../includes/webserv.hpp"

bool	cgi_parser(std::vector<std::string> &tokens,
		std::list<struct ServerConfig> &serverList)
{
	if (tokens.size() != 3 || serverList.empty())
		return true;
	if (serverList.empty() || serverList.back().locations.empty()
		|| serverList.back().locations.back().section_closed == true)
		return true;
	size_t	pos = tokens[2].find(";");
	if (pos != tokens[2].size() - 1)
		return true;
	tokens[2].resize(tokens[2].size() - 1);
	serverList.back().locations.back().cgi.insert(std::make_pair(tokens[1], tokens[2]));
	return false;
}
