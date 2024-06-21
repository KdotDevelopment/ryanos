#pragma once
#include "../graphics/graphics.hpp"
#include "out.hpp"
#include "../memory/mem.hpp"
#include "../memory/page_frame_allocator.hpp"
#include "commands/command_manager.hpp"

#define SHELL_COLOR 0xFFEEEEEE
#define MAX_COMMAND_CHARACTERS 256

class Shell {
	private:
	void draw_toolbar();
	char current_line[MAX_COMMAND_CHARACTERS];

	public:
	//CommandManager cmd_manager;
	Shell();
	void init_shell();
	void execute_command(char *args);
	void handle_keyboard(uint8_t scancode);
};

extern Shell *shell;