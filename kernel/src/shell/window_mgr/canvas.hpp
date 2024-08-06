#pragma once

#include <stdint.h>
#include "../../graphics/point.hpp"
#include "../../graphics/graphics.hpp"
#include "../../graphics/framebuffer.hpp"

class Canvas {
	public:
	Canvas(Point size);

	Point size;
	Framebuffer framebuffer;
	Graphics *gfx;

	void render(Point position);

	void delete_canvas();
};