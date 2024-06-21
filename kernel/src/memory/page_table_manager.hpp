#pragma once

#include "paging.hpp"

class PageTableManager {
	public:
	PageTableManager(PageTable *PML4_address);
	PageTable *PML4_address;
	void map_memory(void *virtual_memory, void *physical_memory);
};

extern PageTableManager page_table_manager;