#include "pit.hpp"
#include "../../io.hpp"

double PIT::time_since_boot = 0;
uint16_t current_divisor = 65535;

void PIT::sleepd(double seconds) {
	double start_time = time_since_boot;
	while(time_since_boot < start_time + seconds) {
		asm("hlt");
	}
}

void PIT::sleep(uint64_t milliseconds) {
	sleepd((double)milliseconds / 1000);
}

void PIT::set_divisor(uint16_t divisor) {
	if(divisor < 100) { //way too fast
		divisor = 100;
	}
	current_divisor = divisor;
	outb(0x40, (uint8_t)(divisor & 0x00FF)); //0x40 = pit chip
	io_wait();
	outb(0x40, (uint8_t)((divisor & 0xFF00) >> 8));
}

uint64_t PIT::get_frequency() {
	return base_frequency / current_divisor;
}

void PIT::set_frequency(uint64_t frequency) {
	set_divisor(base_frequency / frequency);
}

void PIT::tick() {
	time_since_boot += 1 / (double)get_frequency();
}