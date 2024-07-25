#include "../shell.hpp"

void mouse_wait() {
	uint64_t timeout = 100000;
	while(timeout--) {
		if((inb(0x64) & 0b10) == 0) return;
	}
}

void mouse_wait_input() {
	uint64_t timeout = 100000;
	while(timeout--) {
		if(inb(0x64) & 0b1) return;
	}
}

void mouse_write(uint8_t value) {
	mouse_wait();
	outb(0x64, 0xD4);
	mouse_wait();
	outb(0x60, value);
}

uint8_t mouse_read() {
	mouse_wait_input();
	return inb(0x60);
}

uint8_t mouse_cycle = 0;
uint8_t mouse_packet[4];
bool mouse_packet_ready = false;
void Shell::handle_mouse(uint8_t data) {
	switch(mouse_cycle) {
		case 0:
			if(mouse_packet_ready) break;
			if(data & 0b00001000 == 0) break; //if the fourth byte (always 1) isnt 1...
			mouse_packet[0] = data;
			mouse_cycle++;
			break;

		case 1:
			if(mouse_packet_ready) break;
			mouse_packet[1] = data;
			mouse_cycle++;
			break;

		case 2:
			if(mouse_packet_ready) break;
			mouse_packet[2] = data;
			mouse_packet_ready = true;
			mouse_cycle = 0;
			break;
	}
}

void Shell::process_mouse_packet() {
	if(!mouse_packet_ready) return;
	mouse_packet_ready = false;

	bool x_neg, y_neg, x_overflow, y_overflow;
	x_neg = mouse_packet[0] & PS2XSIGN ? true : false;
	y_neg = mouse_packet[0] & PS2YSIGN ? true : false;
	x_overflow = mouse_packet[0] & PS2XOVERFLOW ? true : false;
	y_overflow = mouse_packet[0] & PS2YOVERFLOW ? true : false;

	if(!x_neg) {
		mouse_pos.x += mouse_packet[1];
		if(x_overflow) mouse_pos.x += 255;
	}else {
		mouse_packet[1] = 256 - mouse_packet[1];
		mouse_pos.x -= mouse_packet[1];
		if(x_overflow) mouse_pos.x -= 255;
	}

	if(!y_neg) {
		mouse_pos.y -= mouse_packet[2];
		if(y_overflow) mouse_pos.y -= 255;
	}else {
		mouse_packet[2] = 256 - mouse_packet[2];
		mouse_pos.y += mouse_packet[2];
		if(y_overflow) mouse_pos.y += 255;
	}

	if(mouse_pos.x < 0) mouse_pos.x = 0;
	if(mouse_pos.x > graphics->get_width()-1) mouse_pos.x = graphics->get_width()-1;
	if(mouse_pos.y < 0) mouse_pos.y = 0;
	if(mouse_pos.y > graphics->get_height()-1) mouse_pos.y = graphics->get_height()-1;

	graphics->set_color(0xFFFF0000);
	graphics->draw_pixel(Point(mouse_pos.x, mouse_pos.y));
	out::print((uint64_t)mouse_pos.x);
	out::print(", ");
	out::println((uint64_t)mouse_pos.y);
	mouse_packet_ready = false;
}

void Shell::init_mouse() {
	outb(0x64, 0xA8); //enable auxiliary device (mouse)

	mouse_wait();
	outb(0x64, 0x20); //tells keyboard controller to send command to mouse
	mouse_wait_input();
	uint8_t status = inb(0x60);
	status |= 0b10;
	mouse_wait();
	outb(0x64, 0x60);
	mouse_wait();
	outb(0x60, status); //the "compaq" status byte

	mouse_write(0xF6); //default settings
	mouse_read(); //we dont care what this says

	mouse_write(0xF4);
	mouse_read();
}