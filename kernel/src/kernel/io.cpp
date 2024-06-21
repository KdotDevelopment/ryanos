#include "io.hpp"

void outb(uint16_t port, uint8_t value) {
	asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
	uint8_t return_value;
	asm volatile ("inb %1, %0"
	: "=a"(return_value)
	: "Nd"(port));
	return return_value;
}

void io_wait() {
	asm volatile ("outb %%al, $0x80" : : "a"(0)); //sends 0 to port to waste io cycle to let slow devices to catch up
}