#ifndef SERVER_HPP
# define SERVER_HPP

# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <iostream>
# include <errno.h>
# include <fcntl.h>
# include <iostream>
# include <sys/select.h>
# include <vector>
# include <unordered_map>
# include <unordered_set>
# include <utility>
# include <fstream>
# include <string>
# include <sstream>
# include <signal.h>

typedef struct user {
	int socket;
	std::string nickname;
	std::string username;
	std::vector<std::string> commands;
	bool is_server_op;
	bool is_disconnected;
	bool is_registered;
	bool is_banned;
	bool is_authenticated;
	std::unordered_map<std::string, struct channel *> channels;
} user_t;

typedef struct channel {
	std::string name;
	std::unordered_set<std::string> connected_users;
	std::unordered_set<std::string> operators;
} channel_t;

class Server {
public:
	Server(std::string host, int port, std::string password);

	int waitClient();

	void addClient(int sockfd);

	void listenClients();

private:
	int sockfd;
	std::string host;
	int port;
	int fd_max;

	std::string pass;

	struct fd_set readfds;
	struct fd_set activefds;

	//map of socket fds with user pointers
	std::unordered_map<int, user_t *> users;
	std::unordered_map<std::string, channel_t *> channels;

};

#endif
