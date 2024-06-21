#include "out.hpp"
#include "../lib/cstr.hpp"

Point cursor_pos(0,0);
Point user_begin_pos(4,0);
uint32_t out_color = 0xFF000000;

void out::set_color(uint32_t new_color) {
	out_color = new_color;
}

void out::print(const char *str) {
	uint32_t prev_color = graphics->get_color();
	char *chr = (char *)str;
	while(*chr != 0) {
		if(*chr == '\n') {
			cursor_pos.x = 0;
			cursor_pos.y += 1;
			user_begin_pos.y += 1;
		}else {
			graphics->set_color(SHELL_COLOR);
			graphics->draw_rect(Point((cursor_pos.x * 8) + 5, (cursor_pos.y * 16) + 45),
								Point((cursor_pos.x * 8) + 5 + 8, (cursor_pos.y * 16) + 45 + 16)); //clears the current space so two characters can't overlap
			graphics->set_color(out_color);
			graphics->draw_char(Point((cursor_pos.x * 8) + 5, (cursor_pos.y * 16) + 45), *chr); //don't draw a '\n'!
			cursor_pos.x += 1;
		}
		if(cursor_pos.x + 8 > graphics->get_width()) {
			cursor_pos.x = 0;
			cursor_pos.y += 1;
		}
		chr++; //pointer arith.
	}
	graphics->swap();
	graphics->set_color(prev_color);
}

void out::cprint(char chr) {
	uint32_t prev_color = graphics->get_color();
	if(chr == '\n') {
		cursor_pos.x = 0;
		cursor_pos.y += 1;
		user_begin_pos.y += 1;
	}else {
		graphics->set_color(SHELL_COLOR);
		graphics->draw_rect(Point((cursor_pos.x * 8) + 5, (cursor_pos.y * 16) + 45),
							Point((cursor_pos.x * 8) + 5 + 8, (cursor_pos.y * 16) + 45 + 16)); //clears the current space so two characters can't overlap
		graphics->set_color(out_color);
		graphics->draw_char(Point((cursor_pos.x * 8) + 5, (cursor_pos.y * 16) + 45), chr); //don't draw a '\n'!
		cursor_pos.x += 1;
	}
	if((cursor_pos.x * 8) + 16 > graphics->get_width()) {
		cursor_pos.x = 0;
		cursor_pos.y += 1;
	}
	graphics->swap();
	graphics->set_color(prev_color);
}

void out::set_cursor_pos(int x, int y) {
	cursor_pos.x = x;
	cursor_pos.y = y;
}

Point out::get_cursor_pos() {
	return cursor_pos;
}

Point out::get_cursor_coords() {
	return Point((cursor_pos.x * 8) + 5, (cursor_pos.y * 16) + 45);
}

void out::backspace() {
	if(cursor_pos.x <= user_begin_pos.x && cursor_pos.y == user_begin_pos.y) return; //this is so the user cant delete the "> " or anything before it
	if(cursor_pos.x > 0) cursor_pos.x -= 1;
	else {
		cursor_pos.y -= 1;
		cursor_pos.x = (graphics->get_width() - 10) / 8;
	}
	uint32_t prev_color = graphics->get_color();
	graphics->set_color(SHELL_COLOR);
	graphics->draw_rect(Point((cursor_pos.x * 8) + 5, (cursor_pos.y * 16) + 45),
						Point((cursor_pos.x * 8) + 5 + 8, (cursor_pos.y * 16) + 45 + 16)); //clears the current space so two characters can't overlap
	graphics->set_color(prev_color);
	graphics->swap();
}

void out::init_user() {
	newline();
	print("> ");
	user_begin_pos = cursor_pos; //this is so the user cant delete the "> " or anything before it
}

void out::enter() {
	newline();
	print("> ");
	user_begin_pos = cursor_pos; //this is so the user cant delete the "> " or anything before it
}

void out::newline() {
	print("\n");
}

void out::print(int64_t value) {
	print(to_string(value));
}

void out::print(uint64_t value) {
	print(to_string(value));
}

void out::print(double value, uint8_t decimal_places) {
	print(to_string(value, decimal_places));
}

void out::print(double value) {
	print(to_string(value));
}

void out::hprint(uint64_t hex) {
	print(to_hstring(hex));
}

void out::hprint(uint32_t hex) {
	print(to_hstring(hex));
}

void out::hprint(uint16_t hex) {
	print(to_hstring(hex));
}

void out::hprint(uint8_t hex) {
	print(to_hstring(hex));
}

//--ln--
void out::println(const char *str) {
	print(str);
	print("\n");
}

void out::println(int64_t value) {
	print(to_string(value));
	print("\n");
}

void out::println(uint64_t value) {
	print(to_string(value));
	print("\n");
}

void out::println(double value, uint8_t decimal_places) {
	print(to_string(value, decimal_places));
	print("\n");
}

void out::println(double value) {
	print(to_string(value));
	print("\n");
}

void out::hprintln(uint64_t hex) {
	print(to_hstring(hex));
	print("\n");
}

void out::hprintln(uint32_t hex) {
	print(to_hstring(hex));
	print("\n");
}

void out::hprintln(uint16_t hex) {
	print(to_hstring(hex));
	print("\n");
}

void out::hprintln(uint8_t hex) {
	print(to_hstring(hex));
	print("\n");
}