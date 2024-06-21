#pragma once
#include <stdint.h>

namespace QWERTYKeyboard {

	#define LEFT_SHIFT 0x2A
	#define RIGHT_SHIFT 0x36
	#define ENTER 0x1C
	#define BACKSPACE 0x0E
	#define SPACE 0x39

	extern const char ASCII_table[];
	char translate(uint8_t scancode, bool uppercase);

}