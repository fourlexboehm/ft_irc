# include "../include/Server.hpp"

//todo: find way to cleanly close server
//todo: prevent users with same username from joining server
//todo: remove users from server. Both for removing clients with matching nicknames
//and removing them once the user quits

void Server::executeCommand(user_t *user, const std::string &cmd)
{
	std::cout << user->nickname << " is executing command:" << std::endl;
	std::cout << "<";
	std::cout << UGRN << cmd << CRESET;
	std::cout << ">" << std::endl;
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
	else if (cmd.find("LIST") == 0)
		this->listChannels(user);
	else if (cmd.find("WHO") == 0)
		this->channelWho(user, cmd);
	else if (cmd.find("QUIT") == 0)
	{
		//todo: remove user from irc
	}
	else if (cmd.find("PING") == 0)
	{
		if (cmd.substr(5).compare(this->host) == 1)
		{
			this->sendMessageRPL(user, "PONG", cmd.substr(5));
			this->sendMessageRPL(user, "", "PONG");
		}
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

	// user_to_kick.erase(remove(user_to_kick.begin(), user_to_kick.end(), '\r'), user_to_kick.end());
	std::string reason = cmd.substr(cmd.find(':') + 1);
	if (cmd.find(':') == std::string::npos)
		reason = "No reason";
	channel_t *channel = this->channels[channel_name];
	if (!channel)
	{
		this->sendMessageRPL(user, "403", "Channel " + channel_name + " could not be found.\r");
		return;
	}
	channel_user_t *kickee = channel->users[user_to_kick];
	channel_user_t *kicker = channel->users[user->nickname];
	std::cout << "<" << user_to_kick << ">" << std::endl;
	if (!kickee || !kicker)
	{
		this->sendMessageRPL(user, "441", "User " + user_to_kick + " not found on " + channel->name + ".\r");
		return;
	}
	if (kickee->is_op || !kicker->is_op)
	{
		this->sendMessageRPL(user, "482", "You are not a channel operator\r");
		return;
	}

	partMessage("PART #" + channel_name + " :"  +  kickee->user->nickname + " Kicked For "+ reason, kickee->user);
	return;
}

//void Server::make
//todo:	check user is not already in channel

void Server::joinChannel(user_t *user, const std::string &cmd)
{
	if (cmd.find('#') != 5)
		return;
	std::string channel_name = cmd.substr(5, cmd.length() - 6);
	std::cout << "CHANNEL NAME: " << channel_name << std::endl;
	if (this->channels.find(channel_name) == this->channels.end())
	{
		std::cout << "Creating new Channel" << std::endl;
		channel_t *channel = new channel_t;
		channel->name = channel_name;
		channel_user_t *channel_user = new channel_user_t;
		channel_user->is_op = true;
		channel_user->user = user;
		channel->users.insert(std::pair<std::string, channel_user_t *>(user->nickname, channel_user));
		user->channels.insert(std::pair<std::string, channel_t *>(channel_name, channel));
		this->channels[channel_name] = channel;
		std::cout << "New Channel Created: " << channel_name << std::endl;
		this->sendChannelMsg(user, user, "JOIN", channel_name);
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
		this->sendChannelMsg(user, user, "JOIN", channel_name);
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

//todo: implement itoa so that number of users can be displayed in each channel

void Server::listChannels(user_t *user)
{
	std::string chans;
	for (std::map<std::string, channel_t *>::iterator it = channels.begin(); it != channels.end(); it++)
	{
		int i = 0;
		for (std::map<std::string, channel_user_t *>::iterator it2 = it->second->users.begin(); it2 != it->second->users.end(); it2++)
		{
			i++;
		}
		//implement itoa I guess
		(void)i;
		this->sendMessageRPL(user, "322", it->first + " 1" + "\r");
	}
}

//todo: figure out how this is supposed to work

void Server::channelWho(user_t *user, const std::string &cmd)
{
	if (cmd[4] != '#')
		return;
	std::string channel_name = cmd.substr(4, cmd.find(':') - 5);
	channel_name = channel_name.substr(0, channel_name.length() - 1);
	if (channels.find(channel_name) == channels.end())
		return ;
	channel_t *channel = channels[channel_name];
	if (channel == NULL)
		return;
	for (std::map<std::string, channel_user_t *>::iterator it = channel->users.begin(); it != channel->users.end(); it++)
	{
		std::string user_info;
		if (it->second->is_op)
		{
			user_info = ":" + this->host + " 352 " + user->nickname + " " + channel->name + " ~" + it->first + " " + this->host + " " + it->first + " H@x: 0 " + it->first + "\n";
		}
		else
		{
			user_info = ":" + this->host + " 352 " + user->nickname + " " + channel->name + " ~" + it->first + " " + this->host + " " + it->first + " Hx: 0 " + it->first + "\n";
		}
		std::cout << user_info << std::endl;
		send(user->socket, user_info.c_str(), user_info.length(), MSG_NOSIGNAL);
	}
	std::string final_msg = ":" + this->host + " 315 " + user->nickname + " " + channel->name + ": " + "End of /WHO list\n";
	std::cout << final_msg << std::endl;
	send(user->socket, final_msg.c_str(), final_msg.length(), MSG_NOSIGNAL);
}

void Server::partMessage(const std::string &cmd, user_t *sender)
{
	if (cmd[5] != '#')
		return;
	std::string channel_name = cmd.substr(5, cmd.find(':') - 6);
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
			// if (it->first != sender->nickname)
			// {
				this->sendChannelMsg(sender, it->second->user, "", cmd);
			// }
		}
	}
}

void Server::forwardMessage(const std::string &cmd, user_t *sender)
{
	if (cmd.find(':') == std::string::npos)
	{
		return;
	}
	if (cmd[8] == '#')
	{
		std::string chan = cmd.substr(8, cmd.find(':') - 9);
		std::cout << "CHANNEL NAME: " << chan << std::endl;
		channel_t *c = this->channels[chan];
		if (c == NULL)
			return;
		if (!c->users[sender->nickname])
		{
			c->users.erase(sender->nickname);
			sendMessageRPL(sender, "404", "User is not authorised for this channel");
			return ;
		}
		std::cout << "Users in channel: " << std::endl;
		for (std::map<std::string, channel_user_t *>::iterator it = c->users.begin(); it != c->users.end(); it++)
		{
			std::cout << it->second->user->nickname;
			if (it->second->user->is_authenticated && it->first != sender->nickname && c->users[sender->nickname])
			{
				std::cout << ", Message Forwarded." << std::endl;
				sendChannelMsg(sender, it->second->user, "PRIVMSG", chan + " :" + cmd.substr(cmd.find(':') + 1));
			}
			else
				std::cout << ", Message Not Forwarded." << std::endl;
		}
		std::cout << "End of list." << std::endl;
	} else
	{
		std::string user = cmd.substr(8, cmd.find(':') - 9);
		user_t *u = this->users[user];
		if (u && u->is_authenticated && u != sender)
		{
			sendChannelMsg(sender, u, "", cmd);
			return ;
		}
		sendMessageRPL(sender, "404", "User not found");
	}
}

void Server::execNic(user_t *user, const std::string &cmd)
{
	std::string nickname = cmd.substr(5, cmd.length() - 6);
	if (this->users.find(nickname) == this->users.end())
	{
		sendChannelMsg(user, user, "NICK", cmd.substr(cmd.find(' ') + 1));
		std::string old_nick = user->nickname;
		user->nickname = nickname;
		for (std::map<std::string, channel_t *>::iterator it = user->channels.begin(); it != user->channels.end(); it++)
		{
			channel_user_t *update = it->second->users[old_nick];
			update->user = user;
			it->second->users[user->nickname] = update;
			it->second->users.erase(old_nick);
		}
		this->users.erase(old_nick);
		this->users[nickname] = user;
		this->pre_nick_users.erase(user);
		std::cout << "All Users in Server:" << std::endl;
		for (std::map<std::string, user_t *>::iterator it = this->users.begin(); it != this->users.end(); it++)
		{
			std::cout << it->second->nickname << std::endl;
		}
		std::cout << "End of list." << std::endl;
		std::cout << old_nick;
		std::cout << " is now called ";
		std::cout << user->nickname << std::endl;
	}
	else if (this->users.find(nickname)->second->socket == user->socket)
	{
		return;
	}
	else
	{
		this->sendMessageRPL(user, "433", "Nickname is already in use");
	}
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

