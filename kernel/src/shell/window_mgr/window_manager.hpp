#pragma once

#include "../../graphics/point.hpp"
#include "../input/mouse.hpp"
#include "window.hpp"

class WindowManager {
	private:
	struct WindowNode {
		WindowNode *next;
		WindowNode *prev;
		Window *window;
	}; //Linked List 1, Bad cpp Vector 0 :)

	size_t window_count = 1;

	WindowNode *first_node;

	size_t focus_index = 0;

	void reset_indices();
	int move_to_top(size_t index); //returns the index to the new last one

	public:
	WindowManager();

	void handle(Mouse *mouse);

	Window *create_window(Point initial_pos, Point size);
	void delete_window(size_t index);
};