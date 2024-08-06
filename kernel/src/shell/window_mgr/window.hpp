#pragma once

#include <stdint.h>
#include <stddef.h>
#include "../../graphics/point.hpp"
#include "../../graphics/graphics.hpp"
#include "../input/mouse.hpp"
#include "canvas.hpp"

class Window {
	public:
	Window(Point initial_pos, Point size, size_t index);
	Canvas *canvas;

	Point position;
	Point canvas_position;
	Point size; //size of framebuffer
	Point real_size;
	char *title;

	size_t index; //used for render order/focus (higher = more priority)
	size_t id = 0; //unique id for every window
	bool has_focus = false;

	Mouse *mouse;

	void render();
	bool is_draggable(Point mouse_pos);
	bool hover_window(Point mouse_pos); //used for determining when user clicks on window for focus
	bool hover_exit(Point mouse_pos);
	bool hover_framebuffer(Point mouse_pos);
	void set_relative_mouse_pos(Point mouse_pos);

	void delete_window();
};