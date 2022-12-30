#include "Server.hpp"

/**
 * @brief	When user joins an unencrypted channel, the bot will enter the channel and
 * 			welcome them. If it is a new channel, it will congratulate the user on the
 * 			new channel
 *
 * @param user			{user_t} - User struct of new user to enter channel
 * @param channel_name	{string} - Name of the channel user has joined
 * @param new_channel	{bool} - new channel == true, existing channel == false
 * todo: add stuff for user to do
 * todo: add functionality for help channel
 * todo: check if bot is already in channel
 */

void	Server::join_channel(user_t *user, std::string channel_name, bool new_channel)
{
	if (!user->socket)
		return ;

	user_t *bot = users["FRIENDBOT"];
	executeCommand(bot, "JOIN #" + channel_name + "!"); //extra character needed, don't know why
	if (new_channel)
		bot_msg("#" + channel_name, "You Have Created A New Channel! It is called " + channel_name);
	else
		bot_msg("#" + channel_name, "Welcome " + user->nickname + " to " + channel_name);
	if (!channel_name.compare("help"))
	{
		bot_msg("#" + channel_name, "Here are some things you can do:");
		//todo: add stuff for user to do
	}
}

/**
 * @brief	A message from the bot welcoming the user to the irc server
 *
 * @param user {user_t} - User Object of newly joined user
 */

void Server::welcome_user(user_t *user)
{
	if (!user->socket)
		return ;

	bot_msg(user->nickname, "Hi, " + user->nickname);
	bot_msg(user->nickname, "Welcome to the IRC server!");
	bot_msg(user->nickname, "Need help getting started?");
	bot_msg(user->nickname, "To get a list of commands you can use:");
	bot_msg(user->nickname, ":JOIN #help");
}

/**
 * @brief	Simplifies the bot messaging, adds colour
 *
 * @param reciever	{string} - name of the recipient
 * @param message	{string} - content of the message
 * todo: figure out why bot messages print extra new line
 */

void	Server::bot_msg(std::string reciever, std::string message)
{
	user_t *bot = users["FRIENDBOT"];
	executeCommand(bot, "PRIVMSG " + reciever + " :" BCYN + message + CRESET);
}

/**
 * @brief	Creates a new user for the bot. Currently uses socket 0 as a dummy for
 * 			the sake of simplicity.
 * todo: figure out why commands require an extra character
 */

void	Server::init_bot( void )
{
	user_t *bot = new user_t;
	int	socket = 0;
	addClient(socket);
	*bot = get_pre_nick_user(socket);
	executeCommand(bot, "NICK FRIENDBOT!");
	*bot = get_user(socket);
	bot->is_authenticated = true;
	executeCommand(bot, "JOIN #help!");
}
