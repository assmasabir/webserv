#include "../includes/webserv.hpp"

bool	tokensTree(std::vector<std::string> &tokens,
		std::list<struct ServerConfig> &serverList)
{
	if (tokens[0] == "server")
		return server_parser(tokens, serverList);
	else if (tokens[0] == "interface_port")
		return interface_port_parser(tokens, serverList);
	else if (tokens[0] == "location")
		return location_parser(tokens, serverList);
	else if (tokens[0] == "root")
		return root_parser(tokens, serverList);
	else if (tokens[0] == "allow_methods")
		return allowed_methods_parser(tokens, serverList);
	else if (tokens[0] == "directory_listing")
		return directory_listing_parser(tokens, serverList);
	else if (tokens[0] == "default_type")
		return default_type_parser(tokens, serverList);
	else if (tokens[0] == "default_file")
		return default_file_parser(tokens, serverList);
	else if (tokens[0] == "error_page")
		return error_page_parser(tokens, serverList);
	else if (tokens[0] == "cgi")
		return cgi_parser(tokens, serverList);
	else if (tokens[0] == "upload_directory")
		return upload_directory_parser(tokens, serverList);
	else if (tokens[0] == "}")
		return rightCurlyBrace(tokens, serverList);
	else if (tokens[0] == "redirect")
		return redirect_parser(tokens, serverList);
	else if (tokens[0] == "max_body_size")
		return max_body_size_parser(tokens, serverList);
	else if (tokens[0] == "include")
		return content_type_parser(tokens, serverList);
	else
		return true;
	return false;
}

int	parse_config(char *argv,
		std::list<struct ServerConfig> &serverList)
{
	std::string	config_path(argv);
	// checking if the file exists and if it is a file in the first place
	if (!is_regular_file(config_path))
		return (-1);
	std::ifstream config_file(config_path.c_str());
	if (!config_file.is_open())
		return (-1);
	// reading form the file
	std::string line;
	int	lineIndex = 0;
	while (getline(config_file, line))
	{
		lineIndex++;
		std::vector<std::string>	tokens;
		splitLine(line, tokens);
		if (tokens.empty())
			continue ;
		// parsing tokens and filling the list
		if (tokensTree(tokens, serverList)) // 0 on success
		{
			std::cout << config_path << ":" << lineIndex
				<< ": Error: " << line << "\n";
			config_file.close();
			return (-1) ;
		}
	}
	config_file.close();
	if (serverList.empty())
	{
		std::cout << config_path << ":"
			<< " Error: Expected server block before EOF\n";
		return (-1);
	}
	return (0);
}
