#include "../includes/webserv.hpp"

bool	max_body_size_parser(std::vector<std::string> &tokens,
		std::list<struct ServerConfig> &serverList)
{
	if (tokens.size() != 2)
		return true;
	if (serverList.empty()
		|| (!serverList.back().locations.empty() && serverList.back().locations.back().section_closed == false)
		|| serverList.back().section_closed == true)
		return true;
	char	*endPtr;
	errno = 0;
	long	max_size = strtol(tokens[1].c_str(), &endPtr, 10);
	// std::cout << "max " << max_size << "\n";
	if (errno == ERANGE ||  max_size < 0 || max_size > INT_MAX)
		return true;
	if (*endPtr != ';' && *endPtr != 'K' && *endPtr != 'M'
			&& *endPtr != 'G')
		return true;
	int	power = 1;
	if (*endPtr == ';')
		power = 1;
	else if (*endPtr == 'K')
		power = 1e3;
	else if (*endPtr == 'M')
		power = 1e6;
	else if (*endPtr == 'G')
		power = 1e9;
	if (max_size > INT_MAX / power)
		return true;
	serverList.back().max_body_size = max_size * power;
	return false;
}
