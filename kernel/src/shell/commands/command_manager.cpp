#include "command_manager.hpp"
#include "../../lib/standard.hpp"
#include "../shell.hpp"

CommandManager::CommandManager() {
	
}

void CommandManager::parse_command(char *args) {
	char *argv[MAX_COMMAND_TOKENS];
	int argc = 0;

	while(*args != 0) {
		while(*args == ' ') {
			*args = 0;
			args++;
		}
		if(*args != 0) {
			argv[argc] = args;
			argc++;
		}
		while(*args != 0 && *args != ' ') {
			args++;
		}
		if(*args != 0) {
			*args = 0;
			args++;
		}
	}
	if(argc == 0) return;
	if(compare_string((const char*)argv[0], "echo") == 0) {
		command_echo(argc, argv);
	}else if(compare_string((const char*)argv[0], "clear") == 0) {
		graphics->set_color(SHELL_COLOR);
		graphics->clear_screen();
		out::set_cursor_pos(0, 0);
	}else {
		out::print("Unknown command: ");
		out::println(argv[0]);
	}
	/*for(int i = 0; i < 1; i++){//sizeof(command_array) / sizeof(command_array[0]); i++) {
		Command command = command_array[i];
		//if(command.name == NULL) return;
		//if(compare_string((const char*)argv[0], (const char*)command.name) == 0) out::println("ecgo");
		if(compare_string(argv[0], command.name) == 0) {
			//command.execute(argc, argv);
		}else {
			out::print("Command not found: ");
			out::println(argv[0]);
		}
	}*/
	//out::println(argv[0]);
	//out::println((uint64_t)argc);
}

void CommandManager::command_echo(int argc, char **argv) {
	for(int i = 1; i < argc; i++) {
		out::print((argv[i]));
		out::print(" ");
	}
}