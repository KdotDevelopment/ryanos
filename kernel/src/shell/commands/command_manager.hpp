#pragma once

#include "../out.hpp"

#define MAX_COMMAND_TOKENS 16

typedef void (*callback)(int argc, char **argv);

struct Command {
	char *name;
	char *description;
	callback execute;
};

class CommandManager {
	public:
	CommandManager();
	void parse_command(char *args);
	void command_echo(int argc, char **argv);
};