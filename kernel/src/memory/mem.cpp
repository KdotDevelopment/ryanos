#include "mem.hpp"
#include "page_frame_allocator.hpp"
#include "page_table_manager.hpp"
#include "../kernel/kernel.hpp"

// #define DYNAMIC_MEM_TOTAL_SIZE 10485760//20971520  //20 MiB
// #define DYNAMIC_MEM_NODE_SIZE sizeof(dynamic_mem_node_t) // 16

// static uint8_t dynamic_mem_area[DYNAMIC_MEM_TOTAL_SIZE];
// static dynamic_mem_node_t *dynamic_mem_start;

unsigned long used_memory_total = 0;

// void init_dynamic_mem() {
// 	dynamic_mem_start = (dynamic_mem_node_t *) dynamic_mem_area;
// 	dynamic_mem_start->size = DYNAMIC_MEM_TOTAL_SIZE - DYNAMIC_MEM_NODE_SIZE;
// 	dynamic_mem_start->next = NULL;
// 	dynamic_mem_start->prev = NULL;
// }

void *heap_start;
void *heap_end;
dynamic_mem_node_t *last_node;
size_t heap_length = 0;

void init_heap(void *heap_address, size_t page_count) {
	void *pos = heap_address;
	
	for(size_t i = 0; i < page_count; i++) {
		page_table_manager.map_memory(pos, global_allocator.request_page());
		pos = (void *)((size_t)pos + 0x1000);
	}

	heap_length = page_count * 0x1000;

	heap_start = heap_address;
	heap_end = (void *)((size_t)heap_start + heap_length);
	dynamic_mem_node_t *start_node = (dynamic_mem_node_t *)heap_address;
	start_node->size = heap_length - sizeof(dynamic_mem_node_t);
	start_node->next = NULL;
	start_node->prev = NULL;
	start_node->used = false;
	last_node = start_node;
}

uint64_t get_memory_size(EFI_MEMORY_DESCRIPTOR *m_map, uint64_t m_map_entries, uint64_t m_map_descriptor_size) {
	uint64_t memory_size_bytes = 0;
	//if(memory_size_bytes > 0) return memory_size_bytes;

	for(int i = 0; i < m_map_entries; i++) {
		EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((uint64_t)m_map + (i * m_map_descriptor_size));
		memory_size_bytes += desc->num_pages * 4096;
	}

	return memory_size_bytes;
}

unsigned long get_total_dynamic_memory() {
	return heap_length;
}

unsigned long get_used_dynamic_memory() {
	return used_memory_total;
}

unsigned long get_free_dynamic_memory() {
	return (heap_length) - get_used_dynamic_memory();
}

//Finds the smallest block available for the given size needed
void *find_best_mem_block(dynamic_mem_node_t *dynamic_mem, size_t size) {
	dynamic_mem_node_t *best_mem_block = (dynamic_mem_node_t *) NULL;
	uint32_t best_mem_block_size = heap_length + 1;
	
	dynamic_mem_node_t *current_mem_block = dynamic_mem;
	while(current_mem_block) {
		if((!current_mem_block->used) &&
		   (current_mem_block->size >= (size + sizeof(dynamic_mem_node_t))) &&
		   (current_mem_block->size <= best_mem_block_size)) {
			best_mem_block = current_mem_block;
			best_mem_block_size = current_mem_block->size;
		}

		current_mem_block = current_mem_block->next;
	}
	return best_mem_block;
}

void *merge_next_node_into_current(dynamic_mem_node_t *current_node) {
	if(current_node->next == NULL) return current_node;
	if(current_node->next->used) return current_node;
	
	dynamic_mem_node_t *next_node = current_node->next;

	current_node->size += next_node->size;
	current_node->size += sizeof(dynamic_mem_node_t);
	
	//removes the next node
	current_node->next = next_node->next;

	current_node->next->prev = current_node;

	return current_node;
}

void merge_current_node_into_prev(dynamic_mem_node_t *current_node) {
	if(current_node->prev == NULL) return;
	if(current_node->prev->used) return;

	dynamic_mem_node_t *prev_node = current_node->prev;

	prev_node->size += current_node->size;
	prev_node->size += sizeof(dynamic_mem_node_t);

	//removes current node
	prev_node->next = current_node->next;

	if(current_node->next == NULL) return;

	current_node->next->prev = prev_node;
}

/*
malloc( [////] );

  best_mem_block VVV
[/////] [///] [       ]    [////] ->
[/////] [///] [   ]  ....  [////] ->
[/////] [///] [   ] [////] [////]
    mem_node_allocate ^^^
*/

//This is a void so it can be casted to any type as a pointer
void *malloc(size_t size) {
	if (size % 0x10 > 0){ // it is not a multiple of 0x10
		size -= (size % 0x10);
		size += 0x10;
	}
	dynamic_mem_node_t *best_mem_block = (dynamic_mem_node_t *) find_best_mem_block((dynamic_mem_node_t *)heap_start, size);
	
	if(best_mem_block != NULL) {
		//subtracts the size of what we really need from the larger block
		//best_mem_block->size = size;//best_mem_block->size - size - sizeof(dynamic_mem_node_t);

		//creates a new node after best_mem_block
		dynamic_mem_node_t *mem_node_allocate = (dynamic_mem_node_t *) (((uint8_t *) best_mem_block) + sizeof(dynamic_mem_node_t) + size);
		
		//set the params for this new node
		//we are hijacking the best_mem_block as our new node so it goes forwards instead of backwards
		mem_node_allocate->size = best_mem_block->size - size - sizeof(dynamic_mem_node_t);
		best_mem_block->size = size;
		mem_node_allocate->used = false;
		best_mem_block->used = true;
		mem_node_allocate->next = best_mem_block->next;
		mem_node_allocate->prev = best_mem_block;

		//places this new node in between the one we took memory from and the next one
		if(best_mem_block->next != NULL) {
			best_mem_block->next->prev = mem_node_allocate;
		}
		best_mem_block->next = mem_node_allocate;

		used_memory_total += size + sizeof(dynamic_mem_node_t);

		//return ptr to new memory
		return (void *) ((uint8_t *) mem_node_allocate + sizeof(dynamic_mem_node_t));
	}
	expand_heap(size);
	return malloc(size);
}

/*
free( p4 );

p1      p2    p3    p4     p5
[/////] [///] [   ] [////] [////] ->
[/////] [///] [   ] [    ] [////] ->
[/////] [///] [       ]    [////]
*/

//takes in a void argument so it can be of any type
void free(void *p) {
	if(p == NULL) {
		return;
	}

	//gets the node associated with p
	dynamic_mem_node_t *current_node = (dynamic_mem_node_t *) ((uint8_t *) p - sizeof(dynamic_mem_node_t));
	
	//node was never allocated in the first place
	if(current_node == NULL) {
		return;
	}

	used_memory_total -= current_node->size;

	//allows find_best_mem_block to reallocate this in the future
	current_node->used = false;
	
	//merges a possible free node directly ahead of the current one into the current node
	current_node = (dynamic_mem_node_t *)merge_next_node_into_current(current_node);

	//merges a possible free node directly below the current one into the current node (see comment example above free())
	merge_current_node_into_prev(current_node);
}

void expand_heap(size_t length) {
	if (length % 0x1000) {
		length -= length % 0x1000;
		length += 0x1000;
	}

	size_t page_count = length / 0x1000;
	dynamic_mem_node_t *new_node = (dynamic_mem_node_t *)heap_end;

	for (size_t i = 0; i < page_count; i++){
        page_table_manager.map_memory(heap_end, global_allocator.request_page());
        heap_end = (void*)((size_t)heap_end + 0x1000);
    }
}

void memcopy(void *dest, const void *src, size_t n) {
	// Pointers to the start of the source and destination buffers
	uint64_t* d64 = (uint64_t*)dest;
	const uint64_t* s64 = (const uint64_t*)src;

	// Copy data in 64-bit chunks
	size_t num_ints64 = n / sizeof(uint64_t);
	for (size_t i = 0; i < num_ints64; ++i) {
		d64[i] = s64[i];
	}

	// Handle remaining bytes (if any)
	size_t remaining_bytes = n % sizeof(uint64_t);
	if (remaining_bytes > 0) {
		unsigned char* d_byte = (unsigned char*)(d64 + num_ints64);
		const unsigned char* s_byte = (const unsigned char*)(s64 + num_ints64);
		for (size_t i = 0; i < remaining_bytes; ++i) {
			d_byte[i] = s_byte[i];
		}
	}
}

void memset(void *start, uint8_t value, uint64_t num) {
	for(uint64_t i = 0; i < num; i++) {
		*(uint8_t *)((uint64_t)start + i) = value;
	}
}