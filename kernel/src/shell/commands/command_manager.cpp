#include "command_manager.hpp"
#include "../../lib/standard.hpp"
#include "../../graphics/point.hpp"

CommandManager::CommandManager() {
	
}

int strtoi(const char *s) {
    char *p = (char *)s;
    int nmax = (1ULL << 31) - 1;    /* INT_MAX  */
    int nmin = -nmax - 1;           /* INT_MIN  */
    long long sum = 0;
    char sign = *p;

    if (*p == '-' || *p == '+') p++;

    while (*p >= '0' && *p <= '9') {
        sum = sum * 10 - (*p - '0');
        if (sum < nmin || (sign != '-' && -sum > nmax)) return 0;
        p++;
    }

    if (sign != '-') sum = -sum;

    return (int)sum;
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
		out::clear();
	}else if(compare_string((const char*)argv[0], "window") == 0) {
		shell->make_window();
	}else if(compare_string((const char*)argv[0], "windowcolor") == 0) {
		command_window_color(argc, argv);
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

void CommandManager::command_window_color(int argc, char **argv) {
	if(argc != 3) {
		out::println("Usage: windowcolor <id> <red/green/blue>");
		return;
	}
	Window *window = shell->window_manager->get_from_id(strtoi(argv[1]));
	if(window == NULL) return;
	if(compare_string(argv[2], "red") == 0) {
		window->canvas->gfx->set_color(0xFFFF0000);
		window->canvas->gfx->clear_screen();
	}
	if(compare_string(argv[2], "green") == 0) {
		window->canvas->gfx->set_color(0xFF00FF00);
		window->canvas->gfx->clear_screen();
	}
	if(compare_string(argv[2], "blue") == 0) {
		window->canvas->gfx->set_color(0xFF0000FF);
		window->canvas->gfx->clear_screen();
	}
}