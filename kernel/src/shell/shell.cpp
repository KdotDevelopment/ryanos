#include "shell.hpp"
#include "../memory/mem.hpp"
#include "../lib/cstr.hpp"
#include "../memory/bitmap.hpp"
#include "../kernel/interrupts/pit/pit.hpp"

Shell *shell;

Shell::Shell() : mouse_pos { Point(0,0) } {
	//this->cmd_manager = CommandManager();
}

void Shell::draw_toolbar() {
	graphics->set_color(0xFF222255);
	graphics->draw_rect(Point(0,0), Point(graphics->get_width(), 40));

	graphics->set_color(0xFFFFFFFF);

	graphics->draw_string(Point(5,5), "RyanOS");

	graphics->draw_string(Point(100,4), "used:");
	graphics->draw_string(Point(100,20), "free: ");
	graphics->draw_string(Point(148,4), to_string(global_allocator.get_used_memory() / 1024));
	graphics->draw_string(Point(148,20), to_string(global_allocator.get_free_memory() / 1024));
	graphics->draw_string(Point(210,4), to_string(get_used_dynamic_memory() / 1024));
	graphics->draw_string(Point(210,20), to_string(get_free_dynamic_memory() / 1024));
	graphics->draw_string(Point(500,20), to_string(PIT::time_since_boot));
}

void print_welcome() {
	out::println("=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
	out::println(" RyanOS Shell Testing Space");
	out::println("=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
	out::newline();
}

bool is_cursor_on = false;
Point last_cursor_coord(0,0);

void Shell::init_shell() {
	graphics->set_color(SHELL_COLOR);
	graphics->clear_screen();
	draw_toolbar();
	graphics->swap();

	out::set_cursor_pos(0, 0);

	print_welcome();

	out::init_user();
	memset(current_line, (uint8_t)0, MAX_COMMAND_CHARACTERS);

	/* Every Half Second:
	 *  - Updates cursor
	 * 
	 * Every (roughly) 1/1000 Seconds:
	 *  - Updates toolbar
	 *  - Swaps graphics
	 */

	uint64_t ticks = 0;
	bool last_half_second_state = false;

	while(true) {
		if((size_t)(PIT::time_since_boot * 4) % 2 == 0 && !last_half_second_state) { //every half second
			last_half_second_state = true;
			update_cursor(true);
		}else if((size_t)(PIT::time_since_boot * 4) % 2 != 0 && last_half_second_state) {
			last_half_second_state = false;
		}
		if(last_cursor_coord != out::get_cursor_coords()) {
			update_cursor(false);
		}
		process_mouse_packet();
		draw_toolbar();
		graphics->swap();
		ticks++;
		PIT::sleep(5);
	}

}

void Shell::execute_command(char *args) {
	CommandManager cmd_manager = CommandManager();
	out::newline();
	cmd_manager.parse_command(args);
	out::enter();
	return;
}

void Shell::update_cursor(bool update_state) {
	if(update_state) is_cursor_on = !is_cursor_on;
	if(last_cursor_coord != out::get_cursor_coords()) { //cursor has moved
		graphics->set_color(SHELL_COLOR);
		graphics->draw_rect(Point(last_cursor_coord.x, last_cursor_coord.y + 16), Point(last_cursor_coord.x + 8, last_cursor_coord.y + 18));
	}
	if(is_cursor_on || !update_state) {
		graphics->set_color(0xFF000000);
		graphics->draw_rect(Point(out::get_cursor_coords().x, out::get_cursor_coords().y + 16), Point(out::get_cursor_coords().x + 8, out::get_cursor_coords().y + 18));
	}else {
		graphics->set_color(SHELL_COLOR);
		graphics->draw_rect(Point(out::get_cursor_coords().x, out::get_cursor_coords().y + 16), Point(out::get_cursor_coords().x + 8, out::get_cursor_coords().y + 18));
	}
	last_cursor_coord = out::get_cursor_coords();
}