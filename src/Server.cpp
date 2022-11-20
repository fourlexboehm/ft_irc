#include "../include/Server.hpp"

/// @param host Host ip address.
/// @param port Port to listen.
/// @param server password

Server::Server(std::string host, int port, std::string password)
{
	FD_ZERO(&this->readfds);
	FD_ZERO(&this->activefds);
	this->fd_max = 0;
	if (port < 1024 || port > 49151)
	{
		std::cerr << "Invalid port." << std::endl;
		exit(-1);
	}
	this->host = host;
	this->port = port;
	this->pass = password;
	struct sockaddr_in options = {};

	int newSockFd;
	int opt = 1;
	if ((newSockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		std::cerr << "Error when creating socket." << std::endl;
		exit(-1);
	}
	if (setsockopt(newSockFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "Error when setting socket options." << std::endl;
		close(newSockFd);
		exit(-1);
	}

	// set socket options converting the port to Big Endian
	options.sin_family = AF_INET;
	options.sin_addr.s_addr = INADDR_ANY;
	options.sin_port = htons(this->port);
	inet_aton(this->host.c_str(), (struct in_addr *) &options.sin_addr.s_addr);
	if (bind(newSockFd, (struct sockaddr *) &options, sizeof(options)) == -1)
	{
		std::cerr << "Error when binding socket." << std::endl;
		close(newSockFd);
		exit(-1);
	}
	if (listen(newSockFd, 128) == -1)
	{
		std::cerr << "Error: " << std::strerror(errno) << std::endl;
		close(newSockFd);
		exit(-1);
	}
	fcntl(newSockFd, F_SETFL, O_NONBLOCK);
	this->sockfd = newSockFd;
	this->fd_max = newSockFd;
	FD_SET(newSockFd, &this->activefds);
}

/// @brief Wait for a client connection.
/// @return Client socket file descriptor.

int Server::waitClient()
{
	int client_socket;
	struct sockaddr_in client_address;
	unsigned int addr_len;
	addr_len = sizeof(client_address);
	client_socket = accept(this->sockfd, (struct sockaddr *) &client_address, &addr_len);
	return (client_socket);
}

/// @brief Add a client to the list of users.
/// @param client_socket User socket file descriptor.

void Server::addClient(int client_socket)
{
	this->fd_max = client_socket > this->fd_max ? client_socket : this->fd_max;
	FD_SET(client_socket, &(this->activefds));
	user_t *user = new user_t;
	user->socket = client_socket;
	user->is_authenticated = false;
	user->is_registered = false;
	user->is_banned = false;
	user->is_disconnected = false;

	// this->users.insert(client_socket, user);
	this->users.insert(std::pair<int, user_t *>(client_socket, user));
}

/// @brief Listen the clients.
/// Receive a client's command and execute it.

void Server::listenClients()
{
//	std::list<user_t *>::iterator it;

	//1500 bytes is the max routable packet size https://www.cloudflare.com/learning/network-layer/what-is-mtu/
	char buffer[1500];
	int len = -1;
	this->readfds = this->activefds;
	if (select(this->fd_max + 1, &this->readfds, NULL, NULL, NULL) == -1)
		return;
	for (std::unordered_map<int, user_t *>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if ((*it).second->is_disconnected)
		{
			FD_CLR((*it).second->socket, &this->activefds);
			close((*it).second->socket);
			std::cout << (*it).second->nickname << " left!" << std::endl;
			delete (*it).second;
			this->users.erase(it);
			break;
		}
		{
//			std::list<user_t *>::iterator it;
			for (std::unordered_map<int, user_t *>::iterator it = users.begin(); it != users.end(); ++it)
			{
				std::cout << "user: " << (*it).second->nickname << std::endl;
			}
			std::cout << "--------------------------" << std::endl;
		}
		int client_fd = (*it).second->socket;
		if (FD_ISSET(client_fd, &this->readfds))
		{
			len = -1;
			//todo avoid having to memset the buffer every time
			memset(buffer, 0, 1500);
			len = recv(client_fd, buffer, 1500, MSG_DONTWAIT);
			if (len == 0)
				//todo impl safe exit
				exit(420);
			else if (len > 0)
			{
				std::string buff(buffer);
				std::istringstream buff_stream(buffer);
				// use separator to read lines of the buffer
				//todo does newline in message break this?
				for (std::string line; std::getline(buff_stream, line, '\n');)
					it->second->commands.push_back(line);
				//debugging
				for (size_t i = 0; i < it->second->commands.size(); i++)
				{
					std::cout << "command: " << it->second->commands[i] << std::endl;
				}
//				parseCommand(it->second);
			}
		}
	}
}

