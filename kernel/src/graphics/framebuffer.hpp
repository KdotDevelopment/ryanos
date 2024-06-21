#pragma once

#include <cstddef>

struct Framebuffer {
	void* base_address;
	size_t buffer_size;
	unsigned int width;
	unsigned int height;
	unsigned int pps;
};
