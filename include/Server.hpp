#ifndef SERVER_HPP
# define SERVER_HPP

# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <iostream>
# include <errno.h>
# include <fcntl.h>

#include <sys/select.h>

# include <list>
# include <vector>
# include <map>
# include <utility>

# include <fstream>
# include <string>

#include <signal.h>


# define OP_PASS				"password"



typedef struct user
{
		int						socket;
		std::string 			nickname;
		std::string 			username;
		std::string				buffer;

		bool					is_server_op;
		bool					is_disconnected;
		bool					is_registered;
		bool					is_banned;
		bool					is_authenticated;

		std::map< std::string, struct channel * >	channels;

} user_t;


typedef struct channel
{
		std::string 		name;
		std::list< user_t * > connected_users;
		std::list< user_t * > operators;

} channel_t;


class Server
{
	public:
										Server(std::string host, int port, std::string password);
		int							waitClient();
		void						addClient(int sockfd);
		void						listenClients();

	private:
		int					sockfd;
		std::string			host;
		int					port;
		int					fd_max;

		std::string			pass;

		struct fd_set		readfds;
		struct fd_set		activefds;

		std::list< user * >	users;
		std::map< std::string, channel_t * >	channels;

};



#endif
