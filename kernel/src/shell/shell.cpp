#include "shell.hpp"
#include "../memory/mem.hpp"
#include "../lib/cstr.hpp"
#include "../memory/bitmap.hpp"
#include "../kernel/interrupts/pit/pit.hpp"
#include "../graphics/3d/cube.hpp"

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
	toolbar_canvas->gfx->set_color(0xFF222255);
	toolbar_canvas->gfx->clear_screen();

	toolbar_canvas->gfx->set_color(0xFFFFFFFF);

	toolbar_canvas->gfx->draw_string(Point(5,4), "RyanOS");
	toolbar_canvas->gfx->draw_string(Point(5,20), "DEV");

	toolbar_canvas->gfx->draw_string(Point(100,4), "used:");
	toolbar_canvas->gfx->draw_string(Point(100,20), "free: ");
	toolbar_canvas->gfx->draw_string(Point(148,4), to_string(global_allocator.get_used_memory() / 1024));
	toolbar_canvas->gfx->draw_string(Point(148,20), to_string(global_allocator.get_free_memory() / 1024));
	toolbar_canvas->gfx->draw_string(Point(230,4), to_string(get_used_dynamic_memory() / 1024));
	toolbar_canvas->gfx->draw_string(Point(230,20), to_string(get_free_dynamic_memory() / 1024));
	toolbar_canvas->gfx->draw_string(Point(320,4), to_string(PIT::time_since_boot));
	toolbar_canvas->gfx->draw_string(Point(320,20), "FPS: ");
	toolbar_canvas->gfx->draw_string(Point(360,20), to_string(fps));

	toolbar_canvas->render(Point(0, 0));
}

void print_welcome() {
	out::println("=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
	out::println(" RyanOS Shell Testing Space");
	out::println("=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
	out::newline();
}

bool is_cursor_on = false;
Point last_cursor_coord(0,0);

void Shell::make_window() {
	Window *window = window_manager->create_window(Point(50,50), Point(500, 500));
	out::print("Index: ");
	out::print(window->index);
	out::print(" ID: ");
	out::println(window->id);
}

void Shell::init_shell() {
	window_manager = new WindowManager();

	toolbar_canvas = new Canvas(Point(graphics->get_width(), 40));
	shell_canvas = new Canvas(Point(graphics->get_width(), graphics->get_height() - 40)); //40 for the topbar

	shell_canvas->gfx->set_color(SHELL_COLOR);
	shell_canvas->gfx->clear_screen();
	//memset(shell_canvas->framebuffer.base_address, SHELL_COLOR, shell_canvas->framebuffer.buffer_size);

	out::set_canvas(shell_canvas);

	draw_toolbar();
	shell_canvas->render(Point(0, 40));

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

	//Window *window = window_manager->create_window(Point(100,100), Point(1000, 720));
	//Window *window2 = window_manager->create_window(Point(600,600), Point(600, 200));
	//Window *window3 = window_manager->create_window(Point(900,620), Point(200, 200));

	//Cube cube(window->canvas, {100, 100, 100}, {50, 100, 50});

	//memset(window2->canvas->framebuffer.base_address, 0xFFBBBBFF, window2->canvas->framebuffer.buffer_size);

	start_time = PIT::time_since_boot;
	while(true) {
		frame_start_time = PIT::time_since_boot;

		//------------------------------------------

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

		//Start 3D Graphics Testing
		/*if(window_manager->get_from_id(1) != NULL) {
			window_manager->get_from_id(1)->canvas->gfx->set_color(0xFFFFFFFF);
			window_manager->get_from_id(1)->canvas->gfx->clear_screen();
			window_manager->get_from_id(1)->canvas->gfx->set_color(0xFFFF0000);

			cube.render();
		}*/

		shell_canvas->render(Point(0, 40));
		window_manager->handle(mouse);

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
		shell_canvas->gfx->set_color(SHELL_COLOR);
		shell_canvas->gfx->draw_rect(Point(last_cursor_coord.x, last_cursor_coord.y + 16), Point(last_cursor_coord.x + 8, last_cursor_coord.y + 18));
	}
	if(is_cursor_on || !update_state) {
		shell_canvas->gfx->set_color(0xFF000000);
		shell_canvas->gfx->draw_rect(Point(out::get_cursor_coords().x, out::get_cursor_coords().y + 16), Point(out::get_cursor_coords().x + 8, out::get_cursor_coords().y + 18));
	}else {
		shell_canvas->gfx->set_color(SHELL_COLOR);
		shell_canvas->gfx->draw_rect(Point(out::get_cursor_coords().x, out::get_cursor_coords().y + 16), Point(out::get_cursor_coords().x + 8, out::get_cursor_coords().y + 18));
	}
	last_cursor_coord = out::get_cursor_coords();
}