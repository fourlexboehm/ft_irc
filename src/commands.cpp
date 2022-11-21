# include "../include/Server.hpp"

void Server::executeCommand(user_t *user, const std::string& cmd)
{
	std::cout << "executing command: " << cmd << std::endl;
	if (cmd.find("NICK ") == 0)
		Server::execNic(user, cmd);
	else if (cmd.find("USER ") == 0)
		this->execUser(user, cmd);
	else if (cmd.find("PASS ") == 0
		&&cmd.substr(5).substr(0, cmd.find(" " - 1)) == this->pass)
	{
			 user->is_authenticated = true;
			 std::cout << "PASS command" << std::endl;
	}
	else if (cmd.find("JOIN ") == 0 && user->is_authenticated)
	{
		std::cout << "JOIN command" << std::endl;
	}
	else if (cmd.find("PRIVMSG ") == 0 && user->is_authenticated)
	{
		std::cout << "PRIVMSG command" << std::endl;
	}
	else if (cmd.find("QUIT") == 0)
	{
		std::cout << "QUIT command" << std::endl;
	}
	else if (cmd.find("PING") == 0)
	{
		std::cout << "PING command" << std::endl;
	}
	else if (cmd.find("PONG") == 0)
	{
		std::cout << "PONG command" << std::endl;
	}
	else
	{
		std::cout << "unknown command" << std::endl;
	}
}

void Server::execUser(user_t *user, const std::string &cmd)
{
	user->username = cmd.substr(5).substr(0, cmd.find(" " - 1));
	this->sendMessageRPL(user, "001", "Welcome to the Internet Relay Network " + user->nickname + "!");
}

void Server::execNic(user_t *user, const std::string &cmd)
{ user->nickname = cmd.substr(5, cmd.length() - 1); }

void Server::parseCommands(user_t *user)
{
	while(!user->commands.empty())
	{
		std::string cmd = user->commands.front();
		//todo cr or nl?
		if (cmd[cmd.size() -1] != '\r')
			//this message wasn't complete so we'll handle it later
			return;
	executeCommand(user, cmd);
	user->commands.pop();
	}

}

