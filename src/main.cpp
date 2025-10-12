#include "../includes/server.hpp"

volatile sig_atomic_t g_running = 1;

void  handle_sigint(int signum)
{
	(void)signum;
	g_running = 0;
	std::cout << "Shutting down the server...\n";
}

int	main(int argc, char **argv)
{
	if (argc == 2)
	{
		signal(SIGINT, handle_sigint);
		signal(SIGTERM, handle_sigint);
		std::list<struct ServerConfig>	serverList;
		if (parse_config(argv[1], serverList))
			return (1);
		// display_config_content(serverList);
		try
		{
			Socket	sock(serverList);
			Server  server(sock.get_listener_fds());
			server.run(serverList);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << "\n";
			return (1);
		}
		return (0);
	}
	std::cout << "Usage:\n./webserv [configuration file]\n";
	return (1);
}
