#include "shell.hpp"
#include "../memory/mem.hpp"
#include "../lib/cstr.hpp"
#include "../memory/bitmap.hpp"
#include "window_mgr/window_manager.hpp"
#include "../kernel/interrupts/pit/pit.hpp"

Shell *shell;

Shell::Shell() {
	//this->cmd_manager = CommandManager();
	mouse = (Mouse *)malloc(sizeof(Mouse));
	mouse->pos = Point(0,0);
	mouse->last_pos = Point(0,0);
	mouse->delta_pos = Point(0,0);
	mouse->button_state = MouseState::M_NONE;
}

void Shell::draw_toolbar() {
	graphics->set_color(0xFF222255);
	graphics->draw_rect(Point(0,0), Point(graphics->get_width(), 40));

	graphics->set_color(0xFFFFFFFF);

	graphics->draw_string(Point(5,4), "RyanOS");
	graphics->draw_string(Point(5,20), "DEV");

	graphics->draw_string(Point(100,4), "used:");
	graphics->draw_string(Point(100,20), "free: ");
	graphics->draw_string(Point(148,4), to_string(global_allocator.get_used_memory() / 1024));
	graphics->draw_string(Point(148,20), to_string(global_allocator.get_free_memory() / 1024));
	graphics->draw_string(Point(230,4), to_string(get_used_dynamic_memory() / 1024));
	graphics->draw_string(Point(230,20), to_string(get_free_dynamic_memory() / 1024));
	graphics->draw_string(Point(320,4), to_string(PIT::time_since_boot));
	graphics->draw_string(Point(320,20), "FPS: ");
	graphics->draw_string(Point(360,20), to_string(fps));
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

	graphics->swap();
}

void Shell::start_loop() {
	bool last_half_second_state = false;

	float frame_start_time = 0;
	float current_time = 0;
	float frame_end_time = 0;
	float delta_time = 0;
	float start_time = 0;
	size_t frames;

	/* Every Half Second:
	 *  - Updates cursor
	 * 
	 * Every Tick:
	 *  - Updates toolbar
	 *  - Swaps graphics
	 */

	int window_width = 120;
	int window_height = 70;

	WindowManager window_manager;
	Window *window = window_manager.create_window(Point(100,100), Point(1000, 720));
	Window *window2 = window_manager.create_window(Point(600,600), Point(600, 200));
	Window *window3 = window_manager.create_window(Point(900,600), Point(200, 200));

	memset(window2->canvas->framebuffer.base_address, 0xFFBBBBFF, window2->canvas->framebuffer.buffer_size);

	out::println(window->index);
	out::println(window2->index);

	start_time = PIT::time_since_boot;
	while(true) {
		frame_start_time = PIT::time_since_boot;

		//------------------------------------------

		graphics->set_color(SHELL_COLOR);
		graphics->clear_screen();

		if((size_t)(frame_start_time * 4) % 2 == 0 && !last_half_second_state) { //every half second
			last_half_second_state = true;
			update_cursor(true);
		}else if((size_t)(frame_start_time * 4) % 2 != 0 && last_half_second_state) {
			last_half_second_state = false;
		}
		if(last_cursor_coord != out::get_cursor_coords()) {
			update_cursor(false);
		}

		draw_toolbar();
		process_mouse_packet();

		if(mouse->pos.x < 0) mouse->pos.x = 0;
		if(mouse->pos.y < 0) mouse->pos.y = 0;

		mouse->delta_pos.x = mouse->pos.x - mouse->last_pos.x;
		mouse->delta_pos.y = mouse->pos.y - mouse->last_pos.y;

		/*if(window->is_draggable(mouse->last_pos) && mouse->button_state == M_LEFT) {
			window->position.x += mouse->delta_pos.x;
			window->position.y += mouse->delta_pos.y;
			if(window->position.x + window->real_size.x > graphics->get_width()) window->position.x = graphics->get_width() - window->real_size.x;
			if(window->position.x < 0) window->position.x = 0;
			if(window->position.y + window->real_size.y > graphics->get_height()) window->position.y = graphics->get_height() - window->real_size.y;
			if(window->position.y < 0) window->position.y = 0;
		}*/

		window_manager.handle(mouse);

		graphics->set_color(0xFF000000);
		graphics->draw_mouse_cursor(Point(mouse->pos.x, mouse->pos.y));

		graphics->swap();
		
		mouse->last_pos = mouse->pos;

		//------------------------------------------

		current_time = PIT::time_since_boot;

		frame_end_time = current_time;

		delta_time += frame_end_time - frame_start_time;
		frames++;

		if(current_time - start_time > 1.0) {
			fps = (uint64_t) frames / delta_time;
			frames = 0;
			delta_time = 0;
			memcopy(&start_time, &current_time, sizeof(float)); //FASTER VERSION OF: start_time = current_time;
		}
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