#pragma once

#include <stdint.h>
#include <stddef.h>
#include "../../graphics/point.hpp"
#include "canvas.hpp"

class Window {
	public:
	Window(Point initial_pos, Point size, size_t index);
	Canvas *canvas;

	Point position;
	Point canvas_position;
	Point size; //size of framebuffer
	Point real_size;

	size_t index;

	void render();
	bool is_draggable(Point mouse_pos);
	bool hover_window(Point mouse_pos); //used for determining when user clicks on window for focus
	bool hover_exit(Point mouse_pos);

	void delete_window();
};