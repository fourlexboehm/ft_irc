# include "../include/Server.hpp"

static void executeCommand(user_t *user, std::string cmd)
{
	std::cout << "executing command: " << cmd << std::endl;
	(void)user;
//	std::string token = cmd.substr(0, cmd.find_first_of(' '));
//	switch (token)
//	{
//		case "NICK":
//			exit(5);
//
//	}
}

void parseCommands(user_t *user)
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

