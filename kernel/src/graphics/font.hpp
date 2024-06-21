#pragma once

struct psf1_header_t {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
};

struct psf1_font_t {
	psf1_header_t *psf1_header;
	void *glyph_buffer;
};