#pragma once

#include <stdint.h>
#include "../graphics/point.hpp"
#include "../graphics/graphics.hpp"
#include "shell.hpp"

extern Point cursor_pos;
extern Point user_begin_pos;
extern uint32_t out_color;

namespace out {
	void set_cursor_pos(int x, int y);
	Point get_cursor_pos();
	Point get_cursor_coords();
	void set_color(uint32_t color);
	void newline();
	void backspace();
	void init_user();
	void enter();
	void print(const char *str);
	void cprint(char chr);
	void print(int64_t value);
	void print(uint64_t value);
	void print(double value, uint8_t decimal_places);
	void print(double value);
	void hprint(uint64_t hex);
	void hprint(uint32_t hex);
	void hprint(uint16_t hex);
	void hprint(uint8_t hex);
	void println(const char *str);
	void println(int64_t value);
	void println(uint64_t value);
	void println(double value, uint8_t decimal_places);
	void println(double value);
	void hprintln(uint64_t hex);
	void hprintln(uint32_t hex);
	void hprintln(uint16_t hex);
	void hprintln(uint8_t hex);
};