#include "../includes/webserv.hpp"

bool	allowed_methods_parser(std::vector<std::string> &tokens,
		std::list<struct ServerConfig> &serverList)
{
	if (tokens.size() > 4 || tokens.size() < 2)
		return true;
	if (serverList.empty() || serverList.back().locations.empty()
		|| serverList.back().locations.back().section_closed == true)
		return true;
	size_t	last_token_index = tokens.size() - 1;
	size_t	pos = tokens[last_token_index].find(";");
	if (pos != tokens[last_token_index].size() - 1)
		return true;
	tokens[last_token_index].resize(tokens[last_token_index].size() - 1);
	if (tokens.size() == 2 && tokens[last_token_index].empty())
	{
		serverList.back().locations.back().no_allowed_methods = true;
		return false;
	}
	for (size_t i = 1 ; i < tokens.size() ; i++)
	{
		if (tokens[i] != "GET" && tokens[i] != "POST"
				&& tokens[i] != "DELETE")
			return true;
	}
	for (size_t i = 1 ; i < tokens.size() ; i++)
		serverList.back().locations.back().allowed_methods.push_back(tokens[i]);
	return false;
}
