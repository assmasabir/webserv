#include "../includes/webserv.hpp"

bool	error_page_parser(std::vector<std::string> &tokens,
		std::list<struct ServerConfig> &serverList)
{
	// need to a list for each error page
	if (tokens.size() != 3 || serverList.empty())
		return true;
	if (serverList.empty()
		|| (!serverList.back().locations.empty() && serverList.back().locations.back().section_closed == false)
		|| serverList.back().section_closed == true)
		return true;
	char	*endPtr;
	long	code = strtol(tokens[1].c_str(), &endPtr, 10);
	errno = 0;
	if (errno == ERANGE ||  code < 400 || code > 599 || *endPtr != '\0')
		return true;
	size_t	pos = tokens[2].find(";");
	if (pos != tokens[2].size() - 1)
		return true;
	tokens[2].resize(tokens[2].size() - 1);
	serverList.back().error.insert(std::make_pair(code, tokens[2]));
	return false;
}
