#include "page_table_manager.hpp"
#include "page_map_indexer.hpp"
#include "page_frame_allocator.hpp"
#include "../memory/mem.hpp"
#include <stdint.h>

PageTableManager page_table_manager = NULL;

PageTableManager::PageTableManager(PageTable *PML4_address) {
	this->PML4_address = PML4_address;
}

void PageTableManager::map_memory(void *virtual_memory, void *physical_memory) {
	PageMapIndexer indexer = PageMapIndexer((uint64_t)virtual_memory);
	PageDirectoryEntry PDE;

	PDE = PML4_address->entries[indexer.PDP_i];
	PageTable *PDP;
	if(!PDE.get_flag(PT_FLAG::present)) {
		PDP = (PageTable *)global_allocator.request_page();
		memset(PDP, 0, 0x1000); //0x1000 = 4096, size of page
		PDE.set_address((uint64_t)PDP >> 12);
		PDE.set_flag(PT_FLAG::present, true);
		PDE.set_flag(PT_FLAG::read_write, true);
		PML4_address->entries[indexer.PDP_i] = PDE;
	}else {
		PDP = (PageTable *)((uint64_t)PDE.get_address() << 12);
	}

	PDE = PDP->entries[indexer.PD_i];
	PageTable *PD;
	if(!PDE.get_flag(PT_FLAG::present)) {
		PD = (PageTable *)global_allocator.request_page();
		memset(PD, 0, 0x1000); //0x1000 = 4096, size of page
		PDE.set_address((uint64_t)PD >> 12);
		PDE.set_flag(PT_FLAG::present, true);
		PDE.set_flag(PT_FLAG::read_write, true);
		PDP->entries[indexer.PD_i] = PDE;
	}else {
		PD = (PageTable *)((uint64_t)PDE.get_address() << 12);
	}

	PDE = PD->entries[indexer.PT_i];
	PageTable *PT;
	if(!PDE.get_flag(PT_FLAG::present)) {
		PT = (PageTable *)global_allocator.request_page();
		memset(PT, 0, 0x1000); //0x1000 = 4096, size of page
		PDE.set_address((uint64_t)PT >> 12);
		PDE.set_flag(PT_FLAG::present, true);
		PDE.set_flag(PT_FLAG::read_write, true);
		PD->entries[indexer.PT_i] = PDE;
	}else {
		PT = (PageTable *)((uint64_t)PDE.get_address() << 12);
	}

	PDE = PT->entries[indexer.P_i];
	PDE.set_address((uint64_t)physical_memory >> 12);
	PDE.set_flag(PT_FLAG::present, true);
	PDE.set_flag(PT_FLAG::read_write, true);
	PT->entries[indexer.P_i] = PDE;
}

uint64_t *PageTableManager::get_page_table_entry_from_table(PageTable *table, uint64_t index) {
    PageDirectoryEntry PDE = table->entries[index];
    if (PDE.get_flag(PT_FLAG::present)) {
        return (uint64_t *)(PDE.get_address() << 12);
    } else {
        return NULL;
    }
}

PageDirectoryEntry *PageTableManager::get_page_table_entry(uint64_t virtual_address) {
    PageMapIndexer indexer = PageMapIndexer(virtual_address);

    PageTable *PDP = (PageTable *)get_page_table_entry_from_table(PML4_address, indexer.PDP_i);
    if (!PDP) return NULL;

    PageTable *PD = (PageTable *)get_page_table_entry_from_table(PDP, indexer.PD_i);
    if (!PD) return NULL;

    PageTable *PT = (PageTable *)get_page_table_entry_from_table(PD, indexer.PT_i);
    if (!PT) return NULL;

    return &PT->entries[indexer.P_i];
}