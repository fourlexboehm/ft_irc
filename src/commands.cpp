# include "../include/Server.hpp"

void Server::executeCommand(user_t *user, const std::string &cmd)
{
	std::cout << "executing command:" << std::endl <<
	UGRN << cmd << CRESET << std::endl;
	if (cmd.find("NICK ") == 0)
	{
		Server::execNic(user, cmd);
	}
	else if (cmd.find("USER ") == 0)
		this->execUser(user, cmd);
	else if (cmd.find("PASS ") == 0 && cmd.substr(5, cmd.length() - 6) == this->pass)
		user->is_authenticated = true;
	else if (cmd.find("JOIN ") == 0 && user->is_authenticated)
		this->joinChannel(user, cmd);
	else if (cmd.find("PRIVMSG ") == 0 && user->is_authenticated)
		this->forwardMessage(cmd, user);
	else if (cmd.find("PART") == 0)
		this->partMessage(cmd, user);
	else if (cmd.find("KICK") == 0)
		this->kickUser(cmd, user);
	else if (cmd.find("QUIT") == 0)
	{
	} else if (cmd.find("PING") == 0)
	{
		if (cmd.substr(5) == this->host)
			this->sendMessageRPL(user, "PONG", cmd.substr(5));
//		else
//			this->sendMessageRPL(user, "PONG", "");
	} else if (cmd.find("PONG") == 0)
	{
		std::cout << "PONG command" << std::endl;
	} else
	{
		std::cout << "unknown command" << std::endl;
	}
}

void Server::execUser(user_t *user, const std::string &cmd)
{
	user->username = cmd.substr(5, cmd.find(' ', 5) - 5);
	if (user->is_authenticated)
	{
		this->sendMessageRPL(user, "001", "Welcome to the Internet Relay Network " + user->nickname + "!");
		//bot stuff
		welcome_user(user);
	}
	else
		this->sendMessageRPL(user, "427", "Error, you are not authenticated");
}

//operator commands
void Server::kickUser(const std::string &cmd, user_t *user)
{
	std::string channel_name = cmd.substr(6, cmd.find(' ', 5) - 6);
	std::string user_to_kick = cmd.substr(cmd.find(' ', 5) + 1,
										  cmd.find(' ', cmd.find(' ', 5) + 1) - cmd.find(' ', 5) - 1);

	std::string reason = cmd.substr(cmd.find(':') + 1);
	if (cmd.find(':') == std::string::npos)
		reason = "No reason";
	channel_t *channel = this->channels[channel_name];
	if (!channel)
	{
		this->sendMessageRPL(user, "403", "No such channel");
		return;
	}
	channel_user_t *kickee = channel->users[user_to_kick];
	channel_user_t *kicker = channel->users[user->nickname];
	if (!kickee || !kicker)
	{
		this->sendMessageRPL(user, "441", "They aren't on that channel");
		return;
	}
	if (kickee->is_op || !kicker->is_op)
	{
		this->sendMessageRPL(user, "482", "You are not a channel operator");
		return;
	}

	partMessage("PART #" + channel_name + " :"  +  kickee->user->nickname + " Kicked For "+ reason, kickee->user);
	return;
}

//void Server::make

void Server::joinChannel(user_t *user, const std::string &cmd)
{
	if (cmd.find('#') != 5)
		return;
	std::cout << "Creating new Channel" << std::endl;
	std::string channel_name = cmd.substr(6, cmd.length() - 7);
	if (this->channels.find(channel_name) == this->channels.end())
	{
		channel_t *channel = new channel_t;
		channel->name = channel_name;
		channel_user_t *channel_user = new channel_user_t;
		channel_user->is_op = true;
		channel_user->user = user;
		channel->users.insert(std::pair<std::string, channel_user_t *>(user->nickname, channel_user));
		user->channels.insert(std::pair<std::string, channel_t *>(channel_name, channel));
		this->channels[channel_name] = channel;
		std::cout << "New Channel Created: " << channel_name << std::endl;
		//bot stuff
		this->join_channel(user, channel_name, true);
	} else
	{
		channel_user_t *channel_user = new channel_user_t;
		channel_user->is_op = false;
		channel_user->user = user;
		channel_t *channel = this->channels[channel_name];
		channel->users.insert(std::pair<std::string, channel_user_t *>(user->nickname, channel_user));
		user->channels.insert(std::pair<std::string, channel_t *>(channel_name, channel));
		for (std::map<std::string, channel_user_t *>::iterator it = channel->users.begin();
			 it != channel->users.end(); ++it)
		{
			if (it->first != user->nickname)
			{
				this->sendChannelMsg(user, it->second->user, "JOIN", user->nickname + " " + channel_name);
			}
		}
		std::cout << user->nickname << " Joined Channel: " << channel_name << std::endl;
		//bot stuff
		this->join_channel(user, channel_name, false);
	}
}

void Server::partMessage(const std::string &cmd, user_t *sender)
{
	if (cmd[5] != '#')
		return;
	std::string channel_name = cmd.substr(6, cmd.find(':') - 7);
	if (cmd.find(':') == std::string::npos) {
		channel_name = cmd.substr(6, cmd.length() - 7);
	}
	if (this->channels[channel_name] == NULL)
		return;
	else
	{
		channel_t *channel = this->channels[channel_name];
		channel_user_t *channel_user = channel->users[sender->nickname];
		channel->users.erase(sender->nickname);
		delete channel_user;
		sender->channels.erase(channel_name);
		for (std::map<std::string, channel_user_t *>::iterator it = channel->users.begin();
			 it != channel->users.end(); ++it)
		{
			if (it->first != sender->nickname)
			{
				this->sendChannelMsg(sender, it->second->user, "", cmd);
			}
		}
	}
}

// todo: currently changing nickname while in channel causes a segfault when new message is sent

void Server::forwardMessage(const std::string &cmd, user_t *sender)
{
	if (cmd.find(':') == std::string::npos)
	{
		return;
	}
	if (cmd[8] == '#')
	{
		std::string chan = cmd.substr(9, cmd.find(':') - 10);
		channel_t *c = this->channels[chan];
		if (c == NULL)
			return;
		std::cout << "Users in channel: " << std::endl;
		for (std::map<std::string, channel_user_t *>::iterator it = c->users.begin(); it != c->users.end(); it++)
		{
			std::cout << it->first << std::endl;
			if (it->second->user->is_authenticated && it->first != sender->nickname && c->users[sender->nickname])
			{
				std::cout << "Sending Message to this user." << std::endl;
				sendChannelMsg(sender, it->second->user, "PRIVMSG", "#" + chan + " :" + cmd.substr(cmd.find(':') + 1));
			}
		}
		std::cout << "End of list." << std::endl;
	} else
	{
		std::string user = cmd.substr(8, cmd.find(':') - 9);
		user_t *u = this->users[user];
		if (u && u->is_authenticated && u != sender)
			sendChannelMsg(sender, u, "", cmd);
		else
			sendMessageRPL(sender, "404", "User not found");
	}
}

void Server::execNic(user_t *user, const std::string &cmd)
{
	std::string nickname = cmd.substr(5, cmd.length() - 6);
	std::cout << user->nickname;
	if (this->users.find(nickname) == this->users.end())
	{
		user->nickname = nickname;
		this->users[nickname] = user;
		this->pre_nick_users.erase(user);
	} else if (this->users.find(nickname)->second->socket == user->socket)
	{
		return;
	} else
	{
		this->sendMessageRPL(user, "433", "Nickname is already in use");
	}
	std::cout << " is now called ";
	std::cout << user->nickname << std::endl;
}

void Server::parseCommands(user_t *user)
{
	while (!user->commands.empty())
	{
		std::string cmd = user->commands.front();
		if (cmd[cmd.size() - 1] != '\r')
			//this message wasn't complete, so we'll handle it later
			return;
		executeCommand(user, cmd);
		user->commands.pop();
	}
}

