#include "window.hpp"
#include "../../graphics/graphics.hpp"

Window::Window(Point initial_pos, Point size, size_t index) : position(initial_pos), size(size), real_size(Point(0,0)), canvas_position(Point(initial_pos.x + 1, initial_pos.y + 25)) {
	this->real_size.x = size.x + 2; 
	this->real_size.y = size.y + 26; //24 for top bar, 2 for the 1px border
	this->canvas = new Canvas(size);
	this->index = index;
	this->title = "Window";
	this->mouse = (Mouse *)malloc(sizeof(Mouse));
}

void Window::render() {
	//border
	graphics->set_color(0xFF888888);
	graphics->draw_rect(position, Point(position.x + real_size.x, position.y + real_size.y));

	//top bar
	graphics->set_color(has_focus ? 0xFFAAAAAA : 0xFFCCCCCC);
	graphics->draw_rect(Point(position.x + 1, position.y + 1), Point(position.x + real_size.x - 1, position.y + 24));

	//red X area
	graphics->set_color(has_focus ? 0xFFFF0000 : 0xFF999999);
	graphics->draw_rect(Point(position.x + real_size.x - 60, position.y + 2), Point(position.x + real_size.x - 2, position.y + 23));

	//X character
	graphics->set_color(0xFFFFFFFF);
	graphics->draw_char(Point(position.x + real_size.x - 34, position.y + 5), 'X');

	//framebuffer area
	graphics->set_color(0xFFFFFFFF);
	graphics->draw_rect(Point(position.x + 1, position.y + 25), Point(position.x + real_size.x - 1, position.y + real_size.y - 1));

	//title
	graphics->set_color(0xFF000000);
	graphics->draw_string(Point(position.x + 5, position.y + 5), title);

	canvas_position.x = position.x + 1;
	canvas_position.y = position.y + 25;

	canvas->render(canvas_position);
}

bool Window::is_draggable(Point mouse_pos) {
	return mouse_pos.x >= position.x && mouse_pos.x < position.x + real_size.x - 62 && mouse_pos.y >= position.y && mouse_pos.y < position.y + 24;
}

bool Window::hover_exit(Point mouse_pos) {
	return mouse_pos.x >= position.x + real_size.x - 60 && mouse_pos.x < position.x + real_size.x - 2 + real_size.x && mouse_pos.y >= position.y + 2 && mouse_pos.y < position.y + 23;
}

bool Window::hover_window(Point mouse_pos) {
	return mouse_pos.x >= position.x && mouse_pos.x < position.x + real_size.x && mouse_pos.y >= position.y && mouse_pos.y < position.y + real_size.y;
}

bool Window::hover_framebuffer(Point mouse_pos) {
	return mouse_pos.x >= canvas_position.x && mouse_pos.x < canvas_position.x + size.x && mouse_pos.y >= canvas_position.y && mouse_pos.y < canvas_position.y + size.y;
}

void Window::set_relative_mouse_pos(Point mouse_pos) {
	mouse->last_pos = mouse->pos;
	mouse->pos = Point(canvas_position.x - mouse_pos.x, canvas_position.y - mouse_pos.y);
	mouse->delta_pos.x = mouse->pos.x - mouse->last_pos.x;
	mouse->delta_pos.y = mouse->pos.y - mouse->last_pos.y;
}

void Window::delete_window() {
	canvas->delete_canvas();
	free(mouse);
	canvas = NULL;
}