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
	if (current_node->next != NULL) {
		current_node->next->prev = current_node;
 	}

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
		return (void *) ((uint8_t *) best_mem_block + sizeof(dynamic_mem_node_t));
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

void *aligned_alloc(size_t alignment, size_t size) {
    // Ensure alignment is a power of two and at least sizeof(void*)
    if ((alignment & (alignment - 1)) != 0 || alignment < sizeof(void*)) {
        return NULL;
    }

    // Allocate a block of memory with additional space to store the alignment metadata
    size_t total_size = size + alignment + sizeof(void*);
    void *raw_memory = malloc(total_size);
    if (raw_memory == NULL) {
        return NULL;
    }

    // Align the pointer within the allocated block
    uintptr_t raw_address = (uintptr_t)raw_memory;
    uintptr_t aligned_address = (raw_address + alignment + sizeof(void*)) & ~(alignment - 1);

    // Store the raw address just before the aligned address
    ((uintptr_t*)aligned_address)[-1] = raw_address;

    return (void*)aligned_address;
}

void aligned_free(void *ptr) {
    if (ptr != NULL) {
        // Retrieve the raw address from just before the aligned address
        uintptr_t raw_address = ((uintptr_t*)ptr)[-1];
        free((void*)raw_address);
    }
}

/*void expand_heap(size_t length) {
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
}*/

void expand_heap(size_t length) {
    // Ensure length is a multiple of 0x1000
    if (length % 0x1000) {
        length -= length % 0x1000;
        length += 0x1000;
    }

    size_t page_count = length / 0x1000;
    dynamic_mem_node_t *new_node = (dynamic_mem_node_t *)heap_end;
    void *old_heap_end = heap_end;

    // Map new pages
    for (size_t i = 0; i < page_count; i++) {
        void *new_page = global_allocator.request_page();
        if (new_page == NULL) {
            // Handle page allocation failure
            // Rollback already allocated pages if necessary
            heap_end = old_heap_end;
            return;
        }
        page_table_manager.map_memory(heap_end, new_page);
        heap_end = (void *)((size_t)heap_end + 0x1000);
    }

    // Update the new node's properties
    new_node->size = length - sizeof(dynamic_mem_node_t);
    new_node->used = false;
    new_node->next = NULL;
    new_node->prev = NULL;

    // Link the new node to the existing heap structure
    dynamic_mem_node_t *last_node = (dynamic_mem_node_t *)((uint8_t *)old_heap_end - sizeof(dynamic_mem_node_t));
    if (heap_start == old_heap_end) {
        // This is the first node in the heap
        heap_start = new_node;
    } else {
        // Find the last node in the current heap
        while (last_node->next != NULL) {
            last_node = last_node->next;
        }
        last_node->next = new_node;
        new_node->prev = last_node;
    }

    // Update heap length
    heap_length += length;
}
       

/*void memcopy(void *dest, const void *src, size_t n) {
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
}*/

void memcopy(void *dest, const void *src, size_t n) {
    // Copy data in 128-bit chunks
    if (n >= 16) {
        size_t num_ints128 = n / 16;

        __asm__ __volatile__ (
            "1:                                 \n"
            "movdqu (%1), %%xmm0                \n" // Load 128 bits from src
            "movdqu %%xmm0, (%0)                \n" // Store 128 bits to dest
            "add $16, %0                        \n" // Increment dest by 16
            "add $16, %1                        \n" // Increment src by 16
            "dec %2                             \n" // Decrement counter
            "jnz 1b                             \n" // Loop if counter != 0
            : "+r" (dest), "+r" (src), "+r" (num_ints128)
            :
            : "memory", "xmm0"
        );

        n %= 16;
    }

    // Copy remaining data in 64-bit chunks
    if (n >= 8) {
        size_t num_ints64 = n / 8;

        __asm__ __volatile__ (
            "rep movsq"
            : "+D" (dest), "+S" (src), "+c" (num_ints64)
            :
            : "memory"
        );

        n %= 8;
    }

    // Copy remaining bytes using rep movsb
    if (n > 0) {
        __asm__ __volatile__ (
            "rep movsb"
            : "+D" (dest), "+S" (src), "+c" (n)
            :
            : "memory"
        );
    }
}

/*void memcopy(void *dest, const void *src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    // Align destination pointer to 32-byte boundary
    while (n >= 64 && (uintptr_t)d % 64 != 0) {
        *d++ = *s++;
        n--;
    }

    // Copy data in 256-bit (32-byte) chunks
    if (n >= 32) {
        size_t num_blocks = n / 32;
        size_t remaining = n % 32;

        __asm__ __volatile__ (
            "1:\n\t"
            "vmovups (%1), %%ymm0\n\t" // Load 256 bits from src
            "vmovups %%ymm0, (%0)\n\t" // Store 256 bits to dest
            "add $32, %0\n\t"          // Increment dest by 32
            "add $32, %1\n\t"          // Increment src by 32
            "dec %2\n\t"               // Decrement counter
            "jnz 1b\n\t"               // Loop if counter != 0
            : "+r"(d), "+r"(s), "+r"(num_blocks)
            : 
            : "memory", "ymm0"
        );

        d += (n - remaining); // Adjust pointer after 256-bit copy
        s += (n - remaining);
        n = remaining;
    }

    // Copy remaining data in 128-bit (16-byte) chunks
    if (n >= 16) {
        size_t num_blocks = n / 16;
        size_t remaining = n % 16;

        __asm__ __volatile__ (
            "1:\n\t"
            "movdqu (%1), %%xmm0\n\t"  // Load 128 bits from src
            "movdqu %%xmm0, (%0)\n\t"  // Store 128 bits to dest
            "add $16, %0\n\t"          // Increment dest by 16
            "add $16, %1\n\t"          // Increment src by 16
            "dec %2\n\t"               // Decrement counter
            "jnz 1b\n\t"               // Loop if counter != 0
            : "+r"(d), "+r"(s), "+r"(num_blocks)
            : 
            : "memory", "xmm0"
        );

        d += (n - remaining); // Adjust pointer after 128-bit copy
        s += (n - remaining);
        n = remaining;
    }

    // Copy remaining data in 64-bit (8-byte) chunks
    if (n >= 8) {
        size_t num_blocks = n / 8;
        size_t remaining = n % 8;

        __asm__ __volatile__ (
            "1:\n\t"
            "movsq\n\t"                // Move 64 bits (8 bytes) from src to dest
            "dec %2\n\t"               // Decrement counter
            "jnz 1b\n\t"               // Loop if counter != 0
            : "+D"(d), "+S"(s), "+r"(num_blocks)
            : 
            : "memory"
        );

        d += (n - remaining); // Adjust pointer after 64-bit copy
        s += (n - remaining);
        n = remaining;
    }

    // Copy remaining bytes using rep movsb
    if (n > 0) {
        __asm__ __volatile__ (
            "rep movsb\n\t"            // Move remaining bytes
            : "+D"(d), "+S"(s), "+c"(n)
            : 
            : "memory"
        );
    }
}*/

void memset(void *start, uint32_t c, uint64_t num) {
	uint64_t value64 = (uint64_t)c | ((uint64_t)c << 32);
    uint64_t *dest = (uint64_t *)start;

    // Calculate the number of 64-bit chunks
    uint64_t num_qwords = num / 8;
    uint64_t remaining_bytes = num % 8;

    __asm__ __volatile__ (
        "rep stosq\n"                  // Repeat storing the 64-bit value
        : "=D" (dest), "=c" (num_qwords)  // Output operands
        : "0" (dest), "1" (num_qwords), "a" (value64) // Input operands
        : "memory"                     // Clobbered registers
    );

    // Handle any remaining bytes
    uint8_t *byte_dest = (uint8_t *)dest;
    uint8_t *value_bytes = (uint8_t *)&c;
    for (uint64_t i = 0; i < remaining_bytes; i++) {
        *byte_dest++ = value_bytes[i % 4];
    }
}