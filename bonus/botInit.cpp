#include "Server.hpp"

void	init_bot(Server &server)
{
	user_t *bot = new user_t;
	int	socket = 0;
	server.addClient(socket);
	*bot = server.get_pre_nick_user(socket);
	server.executeCommand(bot, "NICK FRIENDBOT!");
	*bot = server.get_user(socket);
	bot->is_authenticated = true;
	std::cout << bot->is_authenticated << std::endl;
	// std::cout << bot << std::endl;
}