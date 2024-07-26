#include "window.hpp"
#include "../../graphics/graphics.hpp"

Window::Window(Point initial_pos, Point size, size_t index) : position(initial_pos), size(size), real_size(Point(0,0)), canvas_position(Point(initial_pos.x + 1, initial_pos.y + 25)) {
	this->real_size.x = size.x + 26; //25 for top bar, 2 for the 1px border
	this->real_size.y = size.y + 2;  //2 for the 1px border
	this->canvas = new Canvas(size, canvas_position);
	this->index = index;
}

void Window::render() {
	//border
	graphics->set_color(0xFF888888);
	graphics->draw_rect(position, Point(position.x + real_size.x, position.y + real_size.y));

	//top bar
	graphics->set_color(0xFFCCCCCC);
	graphics->draw_rect(Point(position.x + 1, position.y + 1), Point(position.x + real_size.x - 1, position.y + 24));

	//red X area
	graphics->set_color(0xFFFF0000);
	graphics->draw_rect(Point(position.x + real_size.x - 60, position.y + 2), Point(position.x + real_size.x - 2, position.y + 23));

	//X character
	graphics->set_color(0xFFFFFFFF);
	graphics->draw_char(Point(Point(position.x + real_size.x - 34, position.y + 5)), 'X');

	//framebuffer area
	graphics->set_color(0xFFFFFFFF);
	graphics->draw_rect(Point(position.x + 1, position.y + 25), Point(position.x + real_size.x - 1, position.y + real_size.y - 1));

	unsigned int *pix_ptr = (unsigned int *)graphics->backbuffer;
	size_t row_size = ((position.x + real_size.x - 1) - (position.x + 1)) * sizeof(uint32_t);

	for(unsigned int y = position.y + 25; y < position.y + real_size.y - 1; y++) {
		unsigned int *row_start = pix_ptr + (y * graphics->framebuffer->pps) + position.x + 1;
        memcopy(row_start, canvas->framebuffer.base_address, row_size);
	}
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

void Window::delete_window() {
	canvas->delete_canvas();
}