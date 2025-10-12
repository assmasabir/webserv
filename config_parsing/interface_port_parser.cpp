#include "../includes/webserv.hpp"

bool	interface_port_parser(std::vector<std::string> &tokens,
		std::list<struct ServerConfig> &serverList)
{
	if (tokens.size() != 2)
		return true;
	if (serverList.empty()
		|| (!serverList.back().locations.empty() && serverList.back().locations.back().section_closed == false)
		|| serverList.back().section_closed == true)
		return true;
	size_t	pos = tokens[1].find(":");
	if (pos == std::string::npos)
		return true;
	std::string	interface = tokens[1].substr(0, pos);
	std::string	port = tokens[1].substr(pos + 1);
	char	*endPtr;
	errno = 0;
	long	portNumber = strtol(port.c_str(), &endPtr, 10);
	if (errno == ERANGE ||  portNumber < 0 || portNumber > 65535
			|| *(endPtr++) != ';' || *endPtr != '\0')
		return true;
	serverList.back().interface_port.push_back(std::make_pair(portNumber, interface));
	return false;
}
