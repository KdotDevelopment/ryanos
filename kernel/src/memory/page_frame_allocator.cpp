#include "page_frame_allocator.hpp"
#include "../shell/shell.hpp"
#include "page_table_manager.hpp"

bool initialized = false;

uint64_t free_memory;
uint64_t reserved_memory;
uint64_t used_memory;
PageFrameAllocator global_allocator;

void PageFrameAllocator::read_efi_memory_map(EFI_MEMORY_DESCRIPTOR *m_map, size_t m_map_size, size_t m_map_desc_size) {
	if(initialized) return;

	initialized = true;

	uint64_t m_map_entries = m_map_size / m_map_desc_size;

	void *largest_free_mem_seg = NULL;
	size_t largest_free_mem_seg_size = 0;

	for(int i = 0; i < m_map_entries; i++) {
		EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((uint64_t)m_map + (i * m_map_desc_size));
		if(desc->type == 7) { //7 = EfiConventionalMemory, see table
			if(desc->num_pages * 4096 > largest_free_mem_seg_size) {
				largest_free_mem_seg = desc->physical_addr;
				largest_free_mem_seg_size = desc->num_pages * 4096;
			}
		}
	}

	uint64_t memory_size = get_memory_size(m_map, m_map_entries, m_map_desc_size);
	free_memory = memory_size;
	uint64_t bitmap_size = memory_size / 4096 / 8 + 1;

	init_bitmap(bitmap_size, largest_free_mem_seg);

	reserve_pages(0, memory_size / 4096 + 1); //locks entire memory span

	for(int i = 0; i < m_map_entries; i++) {
		EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((uint64_t)m_map + (i * m_map_desc_size));
		if(desc->type == 7) { //EfiConventionalMemory
			unreserve_pages(desc->physical_addr, desc->num_pages); //unreserves everything we can use
		}
	}
	reserve_pages(0, 0x100); //reserves 0 - 0x100000 for bios/uefi stuff
	lock_pages(page_bitmap.buffer, page_bitmap.size / 4096 + 1); //giving the bitmap itself its own space
}

void PageFrameAllocator::init_bitmap(size_t bitmap_size, void *buffer_address) {
	page_bitmap.size = bitmap_size;
	page_bitmap.buffer = (uint8_t *)buffer_address;
	for(int i = 0; i < bitmap_size; i++) {
		*(uint8_t *)(page_bitmap.buffer + i) = 0; //set all bits to 0
	}
}

void PageFrameAllocator::set_page_attribute(void *address, bool write_combining) {
    uint64_t page_address = (uint64_t)address;
    PageDirectoryEntry *pte = page_table_manager.get_page_table_entry(page_address);

    if (pte->get_address()) {
        if (write_combining) {
            *(uint64_t *)pte->value = (*(uint64_t *)pte->value & ~0x3) | 0x01; // Set the PAT index to 1 (Write-Combining)
        } else {
            *(uint64_t *)pte->value = (*(uint64_t *)pte->value & ~0x3) | 0x00; // Set the PAT index to 0 (default Write-Back)
        }
    }
}

uint64_t page_bitmap_index = 0;
void *PageFrameAllocator::request_page() {
	for(; page_bitmap_index < page_bitmap.size * 8; page_bitmap_index++) {
		if(page_bitmap[page_bitmap_index] == true) continue;
		lock_page((void *)(page_bitmap_index * 4096));
		return (void *)(page_bitmap_index * 4096);
	}

	return NULL; //page frame swap to hdd file, ran out of physical mem
}

void *PageFrameAllocator::request_pages(size_t num_pages) {
    uint64_t start_index = page_bitmap_index;
    size_t contiguous_pages = 0;

    for(; page_bitmap_index < page_bitmap.size * 8; page_bitmap_index++) {
        if(page_bitmap[page_bitmap_index] == true) {
            contiguous_pages = 0;
            start_index = page_bitmap_index + 1;
            continue;
        }
        
        if (++contiguous_pages == num_pages) {
            for (size_t i = 0; i < num_pages; i++) {
                lock_page((void *)((start_index + i) * 4096));
            }
            return (void *)(start_index * 4096);
        }
    }

    // No sufficient contiguous pages found
    return NULL; // Page frame swap to HDD file, ran out of physical memory
}

void PageFrameAllocator::free_page(void *address) {
	uint64_t index = (uint64_t)address / 4096;
	if(page_bitmap[index] == false) return; //already free
	if(page_bitmap.set(index, false)) {
		free_memory += 4096;
		used_memory -= 4096;
		if(page_bitmap_index > index) page_bitmap_index = index;
	}
}

void PageFrameAllocator::free_pages(void *address, uint64_t page_count) {
	for(int i = 0; i < page_count; i++) {
		free_page((void *)((uint64_t)address + (i * 4096)));
	}
}

void PageFrameAllocator::lock_page(void *address) {
	uint64_t index = (uint64_t)address / 4096;
	if(page_bitmap[index] == true) return; //already locked
	if(page_bitmap.set(index, true)) {
		free_memory -= 4096;
		used_memory += 4096;
	}
}

void PageFrameAllocator::lock_pages(void *address, uint64_t page_count) {
	for(int i = 0; i < page_count; i++) {
		lock_page((void *)((uint64_t)address + (i * 4096)));
	}
}

void PageFrameAllocator::reserve_page(void *address) {
	uint64_t index = (uint64_t)address / 4096;
	if(page_bitmap[index] == true) return; //already locked
	if(page_bitmap.set(index, true)) {
		free_memory -= 4096;
		reserved_memory += 4096;
	}
}

void PageFrameAllocator::reserve_pages(void *address, uint64_t page_count) {
	for(int i = 0; i < page_count; i++) {
		reserve_page((void *)((uint64_t)address + (i * 4096)));
	}
}

void PageFrameAllocator::unreserve_page(void *address) {
	uint64_t index = (uint64_t)address / 4096;
	if(page_bitmap[index] == false) return; //already free
	if(page_bitmap.set(index, false)) {
		free_memory += 4096;
		reserved_memory -= 4096;
		if(page_bitmap_index > index) page_bitmap_index = index;
	}
}

void PageFrameAllocator::unreserve_pages(void *address, uint64_t page_count) {
	for(int i = 0; i < page_count; i++) {
		unreserve_page((void *)((uint64_t)address + (i * 4096)));
	}
}

uint64_t PageFrameAllocator::get_free_memory() {
	return free_memory;
}

uint64_t PageFrameAllocator::get_used_memory() {
	return used_memory;
}

uint64_t PageFrameAllocator::get_reserved_memory() {
	return reserved_memory;
}