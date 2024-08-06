#include "canvas.hpp"
#include "../../memory/mem.hpp"
#include "../../graphics/graphics.hpp"

Canvas::Canvas(Point size) : size(size) {
	framebuffer.base_address = (uint32_t *)malloc(size.x * size.y * 4);
	memset(framebuffer.base_address, 0xFFFFFFFF, size.x * size.y * 4);
	framebuffer.width = size.x;
	framebuffer.height = size.y;
	framebuffer.pps = size.x;
	framebuffer.buffer_size = framebuffer.width * framebuffer.height * 4;

	this->gfx = new Graphics(true);
	this->gfx->framebuffer = &this->framebuffer;
	this->gfx->set_font(graphics->font);
}

void Canvas::render(Point position) {
	uint32_t *bb_pix_ptr = (uint32_t *)graphics->backbuffer;
	uint32_t *fb_pix_ptr = (uint32_t *)framebuffer.base_address;
	size_t row_size = framebuffer.width * sizeof(uint32_t);

	for(uint32_t y = 0; y < size.y; y++) {
		uint32_t *bb_row_start = bb_pix_ptr + ((y + position.y) * graphics->framebuffer->pps) + position.x;
		uint32_t *fb_row_start = fb_pix_ptr + (y * framebuffer.pps);
       	memcopy(bb_row_start, fb_row_start, row_size);
	}
}

void Canvas::delete_canvas() {
	if(framebuffer.base_address == NULL) return;
	free(framebuffer.base_address);
	free(gfx);
	framebuffer.base_address = NULL;
	free(this);
}