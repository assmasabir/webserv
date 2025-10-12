#include "../includes/webserv.hpp"

bool	insert_extension_path(std::map<std::string, std::string> &m,
		std::ifstream &content_type_file)
{
	std::string	line;
	while (getline(content_type_file, line))
	{
		std::vector<std::string>	tokens;
		splitLine(line, tokens);
		if (tokens.empty())
			continue ;
		if (tokens[0] == "types" && tokens[1] == "{")
			break ;
	}
	while (getline(content_type_file, line))
	{
		std::vector<std::string>	tokens;
		splitLine(line, tokens);
		if (tokens.empty())
			continue ;
		if (tokens[0] == "}")
			break ;
		for (size_t	index = 1 ; index < tokens.size() ; index++)
		{
			if (index == tokens.size() - 1)
			{
				size_t	pos = tokens[index].find(";");
				if (pos != tokens[index].size() - 1)
					return true;
				tokens[index].resize(tokens[index].size() - 1);
			}
			m.insert(std::make_pair(tokens[index], tokens[0]));
		}
	}
	return false;
}

bool	content_type_parser(std::vector<std::string> &tokens,
		std::list<struct ServerConfig> &serverList)
{
	if (tokens.size() != 2)
		return true;
	if (serverList.empty()
		|| (!serverList.back().locations.empty() && serverList.back().locations.back().section_closed == false)
		|| serverList.back().section_closed == true)
		return true;
	size_t	pos = tokens[1].find(";");
	if (pos != tokens[1].size() - 1)
		return true;
	tokens[1].resize(tokens[1].size() - 1);
	std::ifstream	content_type_file(tokens[1].c_str());
	if (!content_type_file.is_open())
		return true;
	if (insert_extension_path(serverList.back().extension_path, content_type_file))
	{
		content_type_file.close();
		return true;
	}
	content_type_file.close();
	if (serverList.back().extension_path.empty())
	{
		std::cout << "Something is wrong with content type file...\n";
		return (true);
	}
	return (false);
}
