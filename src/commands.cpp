# include "../include/Server.hpp"

void Server::executeCommand(user_t *user, const std::string &cmd)
{
	std::cout << "executing command: " << cmd << std::endl;
	if (cmd.find("NICK ") == 0)
		Server::execNic(user, cmd);
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
		this->sendMessageRPL(user, "001", "Welcome to the Internet Relay Network " + user->nickname + "!");
	else
		this->sendMessageRPL(user, "427", "Error, you are not authenticated");
}

void Server::joinChannel(user_t *user, const std::string &cmd)
{
	if (cmd.find('#') != 5)
		return;
	std::string channel_name = cmd.substr(6, cmd.length() - 7);
	if (this->channels.find(channel_name) == this->channels.end())
	{
		channel_t *channel = new channel_t;
		channel->name = channel_name;
		channel->connected_users.insert(user->nickname);
		this->channels[channel_name] = channel;
	} else
	{
		channel_t *channel = this->channels[channel_name];
		channel->connected_users.insert(user->nickname);
		for (std::set<std::string>::iterator it = channel->connected_users.begin();
			 it != channel->connected_users.end(); ++it)
		{
			if (*it != user->nickname)
			{
				this->sendChannelMsg(user, this->users[*it], "JOIN", user->nickname + " " + channel_name);
			}
		}
	}
}

void Server::partMessage(const std::string &cmd, user_t *sender)
{
	if (cmd[5] != '#')
		return;
	std::string channel_name = cmd.substr(6, cmd.find(':') - 7);
	if (this->channels[channel_name] == nullptr)
		return;
	else
	{
		channel_t *channel = this->channels[channel_name];
		channel->connected_users.erase(sender->nickname);
		for (std::set<std::string>::iterator it = channel->connected_users.begin();
			 it != channel->connected_users.end(); ++it)
		{
			if (*it != sender->nickname)
			{
				this->sendChannelMsg(sender, this->users[*it], "PART", sender->nickname + " " + channel_name);
			}
		}
	}
}

void Server::forwardMessage(const std::string &cmd, user_t *sender)
{
	if (cmd[8] == '#')
	{
		std::string chan = cmd.substr(9, cmd.find(':') - 10);
		channel_t *c = this->channels[chan];
		if (c == nullptr) return;
		for (std::set<std::string>::iterator it = c->connected_users.begin(); it != c->connected_users.end(); ++it)
		{
			user_t *u = this->users.find(*it)->second;
			if (u->is_authenticated && u->nickname != sender->nickname)
				sendChannelMsg(sender, u, "PRIVMSG", "#" + chan + " :" + cmd.substr(cmd.find(':') + 1));
		}
	} else
	{
		std::string user = cmd.substr(8, cmd.find(':') - 9);
		user_t *u = this->users[user];
		if (u != nullptr && u->is_authenticated && u != sender)
			sendChannelMsg(sender, u, "", cmd);
	}
}

void Server::execNic(user_t *user, const std::string &cmd)
{
	std::string nickname = cmd.substr(5, cmd.length() - 6);
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
}

void Server::parseCommands(user_t *user)
{
	while (!user->commands.empty())
	{
		std::string cmd = user->commands.front();
		//todo cr or nl?
		if (cmd[cmd.size() - 1] != '\r')
			//this message wasn't complete, so we'll handle it later
			return;
		executeCommand(user, cmd);
		user->commands.pop();
	}
}

