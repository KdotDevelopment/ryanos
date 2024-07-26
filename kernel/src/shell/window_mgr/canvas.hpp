#pragma once

#include <stdint.h>
#include "../../graphics/point.hpp"
#include "../../graphics/framebuffer.hpp"

class Canvas {
	public:
	Canvas(Point size, Point position);

	Point size;
	Point position;
	Framebuffer framebuffer;

	void delete_canvas();
};