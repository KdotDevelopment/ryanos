#include "canvas.hpp"
#include "../../memory/mem.hpp"

Canvas::Canvas(Point size, Point position) : size(size), position(position) {
	framebuffer.base_address = (uint32_t *)malloc(size.x * size.y * 4);
	memset(framebuffer.base_address, 0xFFFFBBBB, size.x * size.y * 4);
	framebuffer.width = size.x;
	framebuffer.height = size.y;
	framebuffer.pps = size.y;
	framebuffer.buffer_size = framebuffer.width * framebuffer.pps * 4;
}

void Canvas::delete_canvas() {
	free(framebuffer.base_address);
}