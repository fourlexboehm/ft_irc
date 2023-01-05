#include <numeric>
#include "../include/Server.hpp"

/// @param host Host ip address.
/// @param port Port to listen.
/// @param server password

Server::Server(const std::string &host, int port, const std::string &password)
{
	std::cout << "Initialising server." << std::endl;
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

	std::cout << BYEL <<
	"Host: " << this->host << std::endl <<
	"Port: " << this->port << std::endl <<
	"Password: " << this->pass << COLOR_RESET <<
	std::endl;

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
	//bot stuff
	init_bot();
}


/// @brief Wait for a client connection.
/// @return Client socket file descriptor.

int Server::waitClient() const
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
	user->nickname = "";

	std::cout <<
	"New User Connected.\n" <<
	"Socket: " << user->socket << std::endl <<
	"Authenticated: " << user->is_authenticated << std::endl <<
	"Registered: " << user->is_registered << std::endl <<
	"Banned: " << user->is_banned << std::endl <<
	"Disconnected: " << user->is_disconnected << std::endl;

	this->pre_nick_users.insert(user);
}



/// @brief Listen the clients.
/// Receive a client's command and execute it.

void Server::handle_client(user_t *it, char buffer[512])
{
	size_t len = -1;
	if (it == NULL)
		return;
	if (it->is_disconnected)
	{
		FD_CLR(it->socket, &this->activefds);
		close(it->socket);
		std::cout << it->nickname << " left!" << std::endl;
		for (std::map<std::string, channel_t *>::iterator it2 = this->channels.begin(); it2 != this->channels.end(); it2++)
		{
			if (it2->second->users.find(it->nickname) != it2->second->users.end())
			{
				channel_user_t *user = it2->second->users[it->nickname];
				std::cout << it->nickname << " Left Channel: " << std::endl;
				delete user;
				it2->second->users.erase(it->nickname);
				//todo delete channel?
//				if (it2->second.users.size() == 0)
//				{
//					this->channels.erase(it2->first);
//					std::cout << "Channel " << it2->first << " deleted." << std::endl;
//				}
			}
		}
		this->users.erase(it->nickname);
		this->pre_nick_users.erase(it);
		delete it;
		return;
	}
	int client_fd = it->socket;
	if (FD_ISSET(client_fd, &this->readfds))
	{
		len = -1;
		//todo avoid having to memset the buffer every time
		memset(buffer, 0, 512);
		len = recv(client_fd, buffer, 512, MSG_DONTWAIT);
		if (len == 0)
			return;
		else
		{
			std::string buff(buffer);
			std::istringstream buff_stream(buffer);
			// use separator to read lines of the buffer
			if (!it->commands.empty())
				std::getline(buff_stream, it->commands.front(), '\n');
			for (std::string line; std::getline(buff_stream, line, '\n');)
				it->commands.push(line);
			parseCommands(it);
		}
	}
}

void Server::listenClients(char buffer[512])
{
	this->readfds = this->activefds;
	if (select(this->fd_max + 1, &this->readfds, NULL, NULL, NULL) == -1)
		return;
	for (std::set<user_t *>::iterator it = pre_nick_users.begin(); it != pre_nick_users.end(); ++it)
	{
		size_t user_count = pre_nick_users.size();
		handle_client(*it, buffer);
		//if the user was deleted, the iterator is invalid
		if (user_count != pre_nick_users.size())
			break;
	}
	for (std::map<std::string, user_t *>::iterator it = users.begin(); it != users.end(); ++it)
	{
		handle_client(it->second, buffer);
	}
}

void Server::sendChannelMsg(user_t *sender, user_t *receiver, std::string rpl_code, std::string message)
{
	std::string hostname = ":" + sender->nickname + "!" + sender->username + "@" + this->host;
	std::string rpl = hostname + " " + rpl_code + " " + message + "\n";
	send(receiver->socket, rpl.c_str(), rpl.length(), MSG_NOSIGNAL);
}

void Server::sendMessageRPL(user_t *user, std::string rpl_code, std::string message)
{
	std::string hostname = ":" + this->host;
	std::string rpl = hostname + " " + rpl_code + " " + user->nickname + " " + message + "\n";
	std::cout << rpl << std::endl;
	send(user->socket, rpl.c_str(), rpl.length(), MSG_NOSIGNAL);
}

//bot stuff

const user_t	&Server::get_pre_nick_user( int socket ) {
	user_t *tmp;
	for (std::set<user_t *>::iterator it = pre_nick_users.begin(); it != pre_nick_users.end(); ++it)
	{
		tmp = *it;
		if (tmp->socket == socket)
			return (*tmp);
	}
	tmp = NULL;
	return (*tmp);
}

const user_t	&Server::get_user( int socket ) {
	user_t *tmp;
	for (std::map<std::string, user_t *>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->second->socket == socket)
		{
			tmp = it->second;
			std::cout << it->second->nickname << std::endl;
			std::cout << "Socket: " << it->second->socket << std::endl;
			return (*tmp);
		}
	}
	tmp = NULL;
	return (*tmp);
}
