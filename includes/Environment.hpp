#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <cstring>

class Environment{
	private:
		std::map<std::string, std::string> env;
	public:
		Environment();
		void 	Add(const std::string key, const std::string value);
		char 	**GetRawEnv();
		void	clear();
		void 	display();
		size_t  GetSize() const { return env.size(); }
		bool    HasVariable(const std::string& key) const;
		std::string GetVariable(const std::string& key) const;
};

void delete_strings(char **env);

#endif