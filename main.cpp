#include "include/Server.hpp"

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "./ircserv <port> <password>" << std::endl;
		return (-1);
	}
	// create a local server instance port and password from arguments
	// todo replace atoi with allowed function
	Server server("127.0.0.1", atoi(argv[1]), std::string(argv[2]));

	int client_socket;
	while (true)
	{
		client_socket = server.waitClient();
		if (client_socket != -1)
			server.addClient(client_socket);
		server.listenClients();
	}
	//todo add safe exit
	return (0);
}