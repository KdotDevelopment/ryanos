#pragma once

#include "paging.hpp"

class PageTableManager {
	public:
	PageTableManager(PageTable *PML4_address);
	PageTable *PML4_address;
	void map_memory(void *virtual_memory, void *physical_memory);
	uint64_t *get_page_table_entry_from_table(PageTable *table, uint64_t index);
	PageDirectoryEntry *get_page_table_entry(uint64_t virtual_address);
};

extern PageTableManager page_table_manager;