#include "../shell.hpp"
#include "scancode_translator.hpp"

bool is_left_shift_pressed;
bool is_right_shift_pressed;

int current_line_index = 0;

void Shell::handle_keyboard(uint8_t scancode) {
	switch(scancode) {
		case LEFT_SHIFT:
			is_left_shift_pressed = true;
			return;
		case LEFT_SHIFT + 0x80:             // 0x80 is added for any release of any key
			is_left_shift_pressed = false;
			return;
		case RIGHT_SHIFT:
			is_right_shift_pressed = true;
			return;
		case RIGHT_SHIFT + 0x80:
			is_right_shift_pressed = false;
			return;
		case ENTER:
			shell->execute_command((char *)current_line);
			current_line_index = 0;
			memset(current_line, 0, MAX_COMMAND_CHARACTERS);
			return;
		case BACKSPACE:
			current_line_index--;
			current_line[current_line_index] = 0;
			out::backspace();
			return;
		case SPACE:
			out::cprint(' ');
			current_line[current_line_index] = ' ';
			current_line_index++;
			return;
	}

	char ascii = QWERTYKeyboard::translate(scancode, is_left_shift_pressed || is_right_shift_pressed);

	if(ascii != 0) {
		out::cprint(ascii);
		current_line[current_line_index] = ascii;
		current_line_index++;
	}
}