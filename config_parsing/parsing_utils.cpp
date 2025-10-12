#include "../includes/webserv.hpp"

bool  is_regular_file(std::string &config_path)
{
	struct stat path_stat;
	if (stat(config_path.c_str(), &path_stat))
	{
		std::cout << "Error: '" << config_path << "' file does not exist.\n";
		return (false);
	}
	if (!S_ISREG(path_stat.st_mode))
	{
		std::cout << "Error: '" << config_path << "' is not a regular file.\n";
		return (false);
	}
	return (true);
}

void	splitLine(std::string &line, std::vector<std::string> &tokens)
{
	std::stringstream	ss(line);
	std::string				token;

	while (ss >> token)
	{
		if (token[0] == '#')
			break ;
		tokens.push_back(token);
	}
}
