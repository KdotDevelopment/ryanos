#include "interrupts.hpp"
#include "../../shell/shell.hpp"
#include "../panic/panic.hpp"
#include "../io.hpp"
#include "pit/pit.hpp"

__attribute__((interrupt)) void page_fault_handler(interrupt_frame *frame) {
	panic((const char*)"Page Fault Detected");
	while(true);
}

__attribute__((interrupt)) void double_fault_handler(interrupt_frame *frame) {
	panic((const char*)"Double Fault Detected");
	while(true);
}

__attribute__((interrupt)) void gp_fault_handler(interrupt_frame *frame) {
	panic((const char*)"General Protection Fault Detected");
	while(true);
}

__attribute__((interrupt)) void keyboard_int_handler(interrupt_frame *frame) {
	uint8_t scancode = inb(0x60); //0x60 = ps/2
	shell->handle_keyboard(scancode);
	pic_end_master();
}

__attribute__((interrupt)) void mouse_int_handler(interrupt_frame *frame) {
	uint8_t mouse_data = inb(0x60);
	out::print("c");
	shell->handle_mouse(mouse_data);
	pic_end_slave();
}

__attribute__((interrupt)) void pit_int_handler(interrupt_frame *frame) {
	PIT::tick();
	pic_end_master();
}

void pic_end_master() {
	outb(PIC1_COMMAND, PIC_EOI);
}

void pic_end_slave() {
	outb(PIC2_COMMAND, PIC_EOI);
	outb(PIC1_COMMAND, PIC_EOI);
}

void remap_pic() {
	uint8_t a1, a2;

	a1 = inb(PIC1_DATA);
	io_wait();
	a2 - inb(PIC2_DATA);
	io_wait();

	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();

	outb(PIC1_DATA, 0x20);
	io_wait();
	outb(PIC2_DATA, 0x28);
	io_wait();

	outb(PIC1_DATA, 4);
	io_wait();
	outb(PIC2_DATA, 2);
	io_wait();

	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, a1);
	io_wait();
	outb(PIC2_DATA, a2);
}