#include "../include/Server.hpp"

/// @brief Create a server.
/// @param host Host ip address.
/// @param port Port to listen.
/// @return The socket file descriptor.

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

	this->host 	= host;
	this->port	= port;
	this->pass	= password;

	struct sockaddr_in	options;

	int	sockfd;
	int	opt 	= 1;

	if ((sockfd	= socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		std::cerr << "Error when creating socket." << std::endl;
		exit(-1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "Error when setting socket options." << std::endl;
		close(sockfd);
		exit(-1);
	}

	// set socket options converting the port to Big Endian
	options.sin_family		= AF_INET;
	options.sin_addr.s_addr = INADDR_ANY;
	options.sin_port		= htons(this->port);

	inet_aton(this->host.c_str(), (struct in_addr *) &options.sin_addr.s_addr);

	if (bind(sockfd, (struct sockaddr *) &options, sizeof(options)) == -1)
	{
		std::cerr << "Error when binding socket." << std::endl;
		close(sockfd);
		exit(-1);
	}

	if (listen(sockfd, 128) == -1)
	{
		std::cerr << "Error: " << std::strerror(errno) << std::endl;
		close(sockfd);
		exit(-1);
	}

	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	this->sockfd = sockfd;
	this->fd_max = sockfd;

	FD_SET(sockfd, &this->activefds);

}

/// @brief Wait for a client connection.
/// @return Client socket file descriptor.

int		Server::waitClient()
{
	int					client_socket;
	struct	sockaddr_in	client_address;
	unsigned int		addr_len; 
	
	addr_len = sizeof(client_address);

	client_socket = accept(this->sockfd, (struct sockaddr *) &client_address, &addr_len);

	return (client_socket);
}

/// @brief Add a client to the list of users.
/// @param client_socket User socket file descriptor.

void	Server::addClient(int client_socket)
{
	this->fd_max = client_socket > this->fd_max ? client_socket : this->fd_max;

	FD_SET(client_socket, &(this->activefds));
	user_t *user = new user_t;
	user->socket = client_socket;

	this->users.push_back(user);
}

/// @brief Listen the clients.
/// Receive a client's command and execute it.

void	Server::listenClients()
{
	std::list< user_t* >::iterator	it;

	char 	buffer[512];
	int		len = -1;

	this->readfds = this->activefds;

	if (select(this->fd_max + 1, &this->readfds, NULL, NULL, NULL) == -1)
		return ;

	for (it = this->users.begin(); it != this->users.end(); it++)
	{
		if ((*it)->is_disconnected)
		{
			FD_CLR((*it)->socket, &this->activefds);
			close((*it)->socket);
			std::cout << (*it)->nickname << " left!" << std::endl;
			delete (*it);
			it = this->users.erase(it);
			break;
		}
		
		{
			std::list< user_t* >::iterator it;

			for (it = this->users.begin(); it != this->users.end(); it++)
			{
				std::cout << "user: " <<(*it)->nickname << std::endl;
			}
			std::cout << "--------------------------" << std::endl;
		}

		int	client_fd = (*it)->socket;
	
		if (FD_ISSET(client_fd, &this->readfds))
		{
			len = -1;

			memset(buffer, 0, 512);

			len = recv(client_fd, buffer, 512, MSG_DONTWAIT);

			if (len == 0)
				exit(420);

			else if (len > 0)
				printf("%s", buffer);
			//todo handle command

				// (*it)->bufferCommand(std::string(buffer), *this);
		}
	}
}


