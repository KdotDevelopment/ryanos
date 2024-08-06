#include "graphics.hpp"
#include "../lib/cstr.hpp"
#include "../memory/mem.hpp"
#include "../lib/math.hpp"

Graphics *graphics;

Graphics::Graphics(bool direct_write) {
	this->framebuffer = NULL;
	this->font = NULL;
	this->direct_write = direct_write;
	//this->backbuffer = NULL;
}

Graphics::Graphics(Framebuffer *framebuffer, psf1_font_t *font) {
	this->framebuffer = framebuffer;
	this->font = font;
	this->width = framebuffer->width;
	this->height = framebuffer->height;
	this->pps = framebuffer->pps;
	//this->backbuffer = { 0 };
	this->backbuffer = (unsigned int *)(malloc(width * height * 4));
	/*for(int i = 0; i < width*height; i++) {
		backbuffer[i] = 0xFF0000FF;
	}*/
}

int Graphics::get_width() {
	return this->width;
}

int Graphics::get_height() {
	return this->height;
}

int Graphics::get_pps() {
	return this->pps;
}

uint32_t Graphics::get_color() {
	return this->color;
}

void Graphics::set_framebuffer(Framebuffer *framebuffer) {
	this->framebuffer = framebuffer;
	this->width = framebuffer->width;
	this->height = framebuffer->height;
	this->pps = framebuffer->pps;
}

void Graphics::set_backbuffer(uint32_t *backbuffer) {
	this->backbuffer = backbuffer;
}

void Graphics::set_font(psf1_font_t *font) {
	this->font = font;
}

void Graphics::set_color(uint32_t color) {
	this->color = color;
}

void Graphics::draw_pixel(Point point) {
	if(point.x > framebuffer->width || point.y > framebuffer->height) return;
	if(direct_write) {
		unsigned int *pix_ptr = (unsigned int *)framebuffer->base_address;
		*(unsigned int *)(pix_ptr + point.x + (point.y * framebuffer->pps)) = color;
		return;
	}
	backbuffer[point.y * pps + point.x] = color;
}

void Graphics::draw_char(Point point, char character) {
	char *font_ptr = (char*)font->glyph_buffer + (character * font->psf1_header->charsize);
	for(unsigned int y = point.y; y < point.y + 16; y++) {
		for(unsigned int x = point.x; x < point.x + 8; x++) {
			if((*font_ptr & (0b10000000 >> (x - point.x))) > 0) {
				draw_pixel(Point(x,y));
			}
		}
		font_ptr++;
	}
}

void Graphics::draw_mouse_cursor(Point point) {
	for(unsigned int y = point.y; y < point.y + 19; y++) {
		for(unsigned int x = point.x; x < point.x + 12; x++) {
			if(y >= framebuffer->pps || x >= framebuffer->width) continue;
			if(mouse_cursor_icon[(y - point.y) * 12 + (x - point.x)] == 1) {
				graphics->set_color(0xFF000000);
				draw_pixel(Point(x,y));
			}
			if(mouse_cursor_icon[(y - point.y) * 12 + (x - point.x)] == 2) {
				graphics->set_color(0xFFFFFFFF);
				draw_pixel(Point(x,y));
			}
		}
	}
}

void Graphics::draw_string(Point point, const char *str) {
	unsigned int x = 0;

	char *chr = (char *)str;
	while(*chr != 0) { //makes sure that the entire next character is within bounds
		draw_char(Point(point.x + (8 * x), point.y), *chr);
		chr++; //pointer arith.
		x++;
	}
}

void Graphics::draw_rect(Point point1, Point point2) {
	if(point1.x > framebuffer->width || point1.y > framebuffer->height) return;
	if(point2.x > framebuffer->width || point2.y > framebuffer->height) return;
	if(point1.x < 0 || point1.y < 0) return;
	if(point2.x < 0 || point2.y < 0) return;

	if(direct_write) {
		uint32_t *pix_ptr = (uint32_t *)framebuffer->base_address;
		size_t row_size = (point2.x - point1.x) * sizeof(uint32_t);

		for(uint32_t y = point1.y; y < point2.y; y++) {
			uint32_t *row_start = pix_ptr + (y * framebuffer->pps) + point1.x;
			memset(row_start, color, row_size);
		}
		return;
	}

	uint32_t *pix_ptr = (uint32_t *)backbuffer;
	size_t row_size = (point2.x - point1.x) * sizeof(uint32_t);

	for(uint32_t y = point1.y; y < point2.y; y++) {
		uint32_t *row_start = pix_ptr + (y * framebuffer->pps) + point1.x;
        memset(row_start, color, row_size);
	}
}

void Graphics::draw_line(Point point1, Point point2) {
	Point d_pos = Point(point2.x - point1.x, point2.y - point1.y);
	float length = sqrt(d_pos.x * d_pos.x + d_pos.y * d_pos.y);
	float angle = atan2(d_pos.y, d_pos.x);

	for(uint64_t i = 0; i < length; i++) {
		draw_pixel(Point(point1.x + cos(angle) * i, 
		                 point1.y + sin(angle) * i));
	}
}

void Graphics::clear_screen() {
	if(direct_write) {
		memset(framebuffer->base_address, color, framebuffer->buffer_size);
		return;
	}
	memset(backbuffer, color, framebuffer->buffer_size);
}

void Graphics::swap() {
	if(direct_write) return; //its already direct, nothing to write!
    memcopy((uint32_t *)framebuffer->base_address, backbuffer, framebuffer->buffer_size);
}