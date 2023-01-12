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
# include "ansi_colours.hpp"

# ifndef MSG_NOSIGNAL
	# define MSG_NOSIGNAL	0x20000
# endif

# define BOTNAME	"FRIENDBOT"

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

typedef struct channel_user {
	user_t *user;
	bool is_op;
} channel_user_t;

typedef struct channel {
	std::string name;
	std::map<std::string, channel_user_t *> users;
} channel_t;

class Server {
public:
	Server(const std::string &host, int port, const std::string &password);

	int waitClient() const;

	void addClient(int sockfd);

	void listenClients(char buffer[512]);

	void sendMessageRPL(user_t *user, std::string rpl_code, std::string message);

	const user_t	&get_pre_nick_user( int socket );

	const user_t	&get_user( int socket );

	void clientCheck( void );

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
	std::set<user_t *> pre_nick_users;
	std::map<std::string , user_t *> users;
	std::map<std::string, channel_t *> channels;

	void execNic(user_t *user, const std::string &cmd);

	void execUser(user_t *user, const std::string &cmd);

	void parseCommands(user_t *user);

	void joinChannel(user_t *user, const std::string &cmd);

	void handle_client(user_t *it, char *buffer);

	void forwardMessage(const std::string &cmd);

	void forwardMessage(const std::string &cmd, user_t *sender);

	void partMessage(const std::string &cmd, user_t *sender);

	void kickUser(const std::string &cmd, user_t *user);

	void executeCommand(user_t *user, const std::string& cmd);

	void sendChannelMsg(user_t *sender, user_t *receiver, std::string rpl_code, std::string message);

	void listChannels(user_t *user);

	void channelWho(user_t *user, const std::string &cmd);

	//bot methods

	void init_bot( void );

	void join_channel(user_t *user, std::string channel_name, bool new_channel);

	void welcome_user(user_t *user);

	void bot_msg(std::string reciever,std::string message);
};

#endif
