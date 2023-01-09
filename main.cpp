#include "Server.hpp"

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "./ircserv <port> <password>" << std::endl;
		return (-1);
	}
	// create a local server instance port and password from arguments
	std::cout << "Connect with /connect localhost" << std::endl;
	Server server("127.0.0.1", std::atoi(argv[1]), std::string(argv[2]));

	int client_socket;
	char buffer[512];
	while (true)
	{
		client_socket = server.waitClient();
		if (client_socket != -1)
			server.addClient(client_socket);
		server.listenClients(buffer);
		// server.clientCheck();
	}
	//todo add safe exit
	return (0);
}
