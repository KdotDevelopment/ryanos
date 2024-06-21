#pragma once

#include <stdint.h>

struct EFI_MEMORY_DESCRIPTOR {
	uint32_t type;
	void *physical_addr;
	void *virtual_addr;
	uint64_t num_pages;
	uint64_t attributes;
};

extern const char *EFI_MEMORY_TYPE_STRINGS[];