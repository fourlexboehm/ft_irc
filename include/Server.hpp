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
# include <map>
# include <set>
# include <utility>
# include <fstream>
# include <string>
# include <stdlib.h>
# include <cstring>
# include <queue>
# include <sstream>
# include <signal.h>
# include "commands.hpp"

typedef struct user {
	int socket;
	std::string nickname;
	std::string username;
	std::queue<std::string> commands;
	bool is_server_op;
	bool is_disconnected;
	bool is_registered;
	bool is_banned;
	bool is_authenticated;
	std::map<std::string, struct channel *> channels;
} user_t;

typedef struct channel {
	std::string name;
	std::set<std::string> connected_users;
	std::set<std::string> operators;
} channel_t;

class Server {
public:
	Server(const std::string &host, int port, const std::string &password);

	int waitClient() const;

	void addClient(int sockfd);

	void listenClients(char buffer[512]);

	void sendMessageRPL(user_t *user, std::string rpl_code, std::string message);

private:
	int sockfd;
	std::string host;
	int port;
	int fd_max;

	std::string pass;

//	struct fd_set readfds;
	fd_set readfds;
//	struct fd_set activefds;
	fd_set activefds;

	//map of socket fds with user pointers
	std::map<int, user_t *> users;
	std::map<std::string, channel_t *> channels;

	static void execNic(user_t *user, const std::string &cmd);

	void executeCommand(user_t *user, const std::string& cmd);

	void execUser(user_t *user, const std::string &cmd);

	void parseCommands(user_t *user);
};

#endif
