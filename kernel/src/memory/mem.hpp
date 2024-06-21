#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../kernel/efi_memory.hpp"

//Makes a linked list
struct dynamic_mem_node_t {
	uint32_t size;
	bool used;
	struct dynamic_mem_node_t *next;
	struct dynamic_mem_node_t *prev;
};

uint64_t get_memory_size(EFI_MEMORY_DESCRIPTOR* m_map, uint64_t m_map_entries, uint64_t m_map_descriptor_size);

//void init_dynamic_mem();
void init_heap(void *heap_address, size_t page_count);

void *malloc(size_t size);
void free(void *p);

void expand_heap(size_t length);

unsigned long get_total_dynamic_memory();
unsigned long get_used_dynamic_memory();
unsigned long get_free_dynamic_memory();

void memcopy(void *dest, const void *src, size_t n);
void memset(void *start, uint8_t value, uint64_t num);