#include "graphics.hpp"
#include "../lib/cstr.hpp"
#include "../memory/mem.hpp"

Graphics *graphics;

Graphics::Graphics() {
	this->framebuffer = NULL;
	this->font = NULL;
	//this->backbuffer = { 0 };
}

Graphics::Graphics(Framebuffer *framebuffer, psf1_font_t *font) {
	this->framebuffer = framebuffer;
	this->font = font;
	this->width = framebuffer->width;
	this->height = framebuffer->height;
	this->pps = framebuffer->pps;
	//this->backbuffer = { 0 };
	this->backbuffer = (unsigned int*)malloc(width * height * 4);
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
}

void Graphics::set_font(psf1_font_t *font) {
	this->font = font;
}

void Graphics::set_color(uint32_t color) {
	this->color = color;
}

void Graphics::draw_pixel(Point point) {
	if(point.x > framebuffer->width || point.y > framebuffer->height) return;
	unsigned int *pix_ptr = (unsigned int *)framebuffer->base_address;
	//*(unsigned int *)(pix_ptr + point.x + (point.y * framebuffer->pps)) = color;
	backbuffer[point.y * pps + point.x] = color;
}

void Graphics::draw_char(Point point, char character) {
	unsigned int *pix_ptr = (unsigned int *)framebuffer->base_address;
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
	unsigned int *pix_ptr = (unsigned int *)framebuffer->base_address;
	for(unsigned int x = point1.x; x < point2.x; x++) {
		for(unsigned int y = point1.y; y < point2.y; y++) {
			//draw_pixel(Point(x, y), color);
			//*(unsigned int *)(pix_ptr + x + (y * framebuffer->pps)) = color;
			backbuffer[y * pps + x] = color;
		}
	}
}

void Graphics::clear_screen() {
	for(int x = 0; x < get_width(); x++){
		for(int y = 0; y < get_height(); y++) {
			backbuffer[y * pps + x] = color;
		}
	}
	/*uint64_t fb_base = (uint64_t)framebuffer->base_address;
    uint64_t bpsl = framebuffer->pps * 4;
    uint64_t fb_height = framebuffer->height;
    uint64_t fb_size = framebuffer->buffer_size;

    for (int vertical_scanline = 0; vertical_scanline < fb_height; vertical_scanline ++){
        uint64_t pix_ptr_base = fb_base + (bpsl * vertical_scanline);
        for (uint32_t* pix_ptr = (uint32_t*)pix_ptr_base; pix_ptr < (uint32_t*)(pix_ptr_base + bpsl); pix_ptr ++){
            *pix_ptr = color;
        }
    }*/
	//memset(backbuffer, color, framebuffer->buffer_size);
}

void Graphics::swap() {
	//if (framebuffer == NULL || backbuffer == NULL) return;
	/*uint64_t base = (uint64_t)framebuffer->base_address;
	uint64_t bpsl = framebuffer->pps * 4; //Bytes per scan line
	uint64_t height = framebuffer->height;

	for(int verticalScanline = 0; verticalScanline < height; verticalScanline++) {
		uint64_t pix_ptr_base = base + (bpsl * verticalScanline);
		uint32_t *back_pix_ptr = (uint32_t *)(*backbuffer + (bpsl * verticalScanline));
		for(uint32_t *pix_ptr = (uint32_t *)pix_ptr_base; pix_ptr < (uint32_t *)(pix_ptr_base + bpsl); pix_ptr++) {
			*pix_ptr = *backbuffer++;
		}
	}*/
	uint32_t *pix_ptr = (uint32_t *)framebuffer->base_address;
    memcopy(pix_ptr, backbuffer, framebuffer->buffer_size);
}