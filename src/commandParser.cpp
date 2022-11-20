# include "../include/Server.hpp"

void parseCommands(user_t *user)
{
	while(!user->commands.empty())
	{
		std::string cmd = user->commands.front();
		//todo cr or nl?
		if (cmd.back() != '\r')
			//this message wasn't complete so we'll handle it later
			return;
//	executeCommand(user, cmd);
	user->commands.pop();
	}

}

