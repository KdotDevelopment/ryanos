#include "paging.hpp"

void PageDirectoryEntry::set_flag(PT_FLAG flag, bool enabled) {
	uint64_t bit_selector = (uint64_t)1 << flag;
	value &= ~bit_selector;
	if(enabled) {
		value |= bit_selector;
	}
}

bool PageDirectoryEntry::get_flag(PT_FLAG flag) {
	uint64_t bit_selector = (uint64_t)1 << flag;
	return value & bit_selector > 0 ? true : false;
}

uint64_t PageDirectoryEntry::get_address() {
	return (value & 0x000ffffffffff000) >> 12; //one f = 4 bits, 10 f's = 40 bits for the address
}

void PageDirectoryEntry::set_address(uint64_t address) {
	address &= 0x000000ffffffffff;
	value &= 0xfff0000000000fff; //clears address from value
	value |= (address << 12);
}