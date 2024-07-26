#pragma once
#include "../graphics/graphics.hpp"
#include "out.hpp"
#include "../kernel/io.hpp"
#include "../memory/mem.hpp"
#include "../memory/page_frame_allocator.hpp"
#include "commands/command_manager.hpp"
#include "input/mouse.hpp"

#define SHELL_COLOR 0xFFEEEEEE
#define MAX_COMMAND_CHARACTERS 256

#define PS2LEFTBTN 0b00000001
#define PS2MIDDLEBTN 0b00000010
#define PS2RIGHTBTN 0b00000100
#define PS2XSIGN 0b00010000
#define PS2YSIGN 0b00100000
#define PS2XOVERFLOW 0b01000000
#define PS2YOVERFLOW 0b10000000

class Shell {
	private:
	void draw_toolbar();
	char current_line[MAX_COMMAND_CHARACTERS];
	void update_cursor(bool update_state);
	Mouse *mouse;
	uint64_t fps;
	uint32_t *shell_backbuffer;

	public:
	//CommandManager cmd_manager;
	Shell();
	void init_shell();
	void start_loop();
	void execute_command(char *args);
	void handle_keyboard(uint8_t scancode);
	void init_mouse();
	void handle_mouse(uint8_t data);
	void process_mouse_packet();
};

extern Shell *shell;