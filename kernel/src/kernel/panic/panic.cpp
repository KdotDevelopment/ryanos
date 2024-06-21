#include "panic.hpp"
#include "../../shell/out.hpp"
#include "../../graphics/point.hpp"

void panic(const char *panic_message) {
	graphics->set_color((uint32_t)0xFF880022);
	graphics->clear_screen();

	out::set_cursor_pos(5, 5);
	out::println("Kernel Panic!");
	out::newline();
	out::newline();

	out::println(panic_message);
}