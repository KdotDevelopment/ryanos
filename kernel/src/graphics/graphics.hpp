#pragma once
#include "framebuffer.hpp"
#include "font.hpp"
#include "point.hpp"
#include "../memory/page_frame_allocator.hpp"
#include <stdint.h>

class Graphics {
	private:
	psf1_font_t *font;
	uint32_t color;
	int width, height, pps;

	public:
	Graphics();
	Graphics(Framebuffer *framebuffer, psf1_font_t *font);

	uint32_t *backbuffer;
	Framebuffer *framebuffer;

	int get_width();
	int get_height();
	int get_pps();
	uint32_t get_color();

	void set_framebuffer(Framebuffer *framebuffer);
	void set_font(psf1_font_t *font);
	void set_color(uint32_t color);
	void draw_pixel(Point point);
	void draw_char(Point point, char character);
	void draw_string(Point point, const char *str);
	void draw_rect(Point point1, Point point2);
	void clear_screen();

	void swap();
};

extern Graphics *graphics;