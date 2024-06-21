#pragma once
#include <stdint.h>

#define IDT_TA_INTERRUPT_GATE 0b10001110
#define IDT_TA_CALLGATE 0b10001100
#define IDT_TA_TRAPGATE 0b10001111

struct IDTDescEntry {
	uint16_t offset0;
	uint16_t selector;
	uint8_t ist;
	uint8_t type_attr;
	uint16_t offset1;
	uint32_t offset2;
	uint32_t ignore;
	void set_offset(uint64_t offset);
	uint64_t get_offset();
};

struct IDTR {
	uint16_t limit;
	uint64_t offset;
} __attribute__((packed));