#pragma once

#include "../kernel/efi_memory.hpp"
#include <stdint.h>
#include "bitmap.hpp"
#include "mem.hpp"
#include "../lib/cstr.hpp"

class PageFrameAllocator {
	private:
	void init_bitmap(size_t bitmap_size, void *buffer_address);
	void reserve_page(void *address);
	void unreserve_page(void *address);
	void reserve_pages(void *address, uint64_t page_count);
	void unreserve_pages(void *address, uint64_t page_count);

	public:
	Bitmap page_bitmap;
	
	void read_efi_memory_map(EFI_MEMORY_DESCRIPTOR *m_map, size_t m_map_size, size_t m_map_desc_size);
	void set_page_attribute(void *address, bool write_combining);
	void free_page(void *address);
	void lock_page(void *address);
	void free_pages(void *address, uint64_t page_count);
	void lock_pages(void *address, uint64_t page_count);
	void *request_page();
	void *request_pages(size_t num_pages);
	uint64_t get_free_memory();
	uint64_t get_used_memory();
	uint64_t get_reserved_memory();
};

extern PageFrameAllocator global_allocator;