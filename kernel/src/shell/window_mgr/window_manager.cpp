#include "window_manager.hpp"

#include "../../memory/mem.hpp"
#include "../../graphics/graphics.hpp"

WindowManager::WindowManager() {
	first_node = (WindowNode *)malloc(sizeof(WindowNode));
	first_node->next = NULL;
	first_node->prev = NULL;
	first_node->window = NULL;
}

Window *WindowManager::create_window(Point initial_pos, Point size) {
	WindowNode *new_node = (WindowNode *)malloc(sizeof(WindowNode));
	WindowNode *window_index = first_node;
	window_count++;
	
	while(window_index->next != NULL) {
		window_index = window_index->next;
	}

	window_index->next = new_node;
	new_node->next = NULL;
	new_node->prev->next = new_node;
	new_node->prev = window_index;
	new_node->window = new Window(initial_pos, size, window_count);
	new_node->window->index = 1; //will be reset anyway

	reset_indices();

	return new_node->window;
}

MouseState prev_mouse_state = MouseState::M_NONE;

void WindowManager::handle(Mouse *mouse) {
	WindowNode *window_index = first_node;

	while(window_index != NULL) {
		if(window_index == first_node) {
			window_index = window_index->next;
			continue;
		}

		if(window_index->window->hover_window(mouse->last_pos) && mouse->button_state == M_LEFT && prev_mouse_state != mouse->button_state) { //determine focus, only fires once
			WindowNode *window_index2 = first_node;

			//Finds the highest index (finds the window on top so you dont focus on a window below)
			while(window_index2->next != NULL) {
				window_index2 = window_index2->next;
				if(window_index2->window->hover_window(mouse->last_pos)) {
					focus_index = window_index2->window->index;
				}
			}
			focus_index = move_to_top(focus_index);
		}

		if(window_index->window->hover_exit(mouse->last_pos) && mouse->button_state == M_LEFT && prev_mouse_state != mouse->button_state && focus_index == window_index->window->index) {
			delete_window(window_index->window->index);
			window_index = window_index->next;
			continue; //dont render it!
		}

		if(window_index->window->is_draggable(mouse->last_pos) && mouse->button_state == M_LEFT && focus_index == window_index->window->index) {
			window_index->window->position.x += mouse->delta_pos.x;
			window_index->window->position.y += mouse->delta_pos.y;
			if(window_index->window->position.x + window_index->window->real_size.x > graphics->get_width()) window_index->window->position.x = graphics->get_width() - window_index->window->real_size.x;
			if(window_index->window->position.x < 0) window_index->window->position.x = 0;
			if(window_index->window->position.y + window_index->window->real_size.y > graphics->get_height()) window_index->window->position.y = graphics->get_height() - window_index->window->real_size.y;
			if(window_index->window->position.y < 0) window_index->window->position.y = 0;
		}

		window_index->window->render();
		window_index = window_index->next;
	}

	prev_mouse_state = mouse->button_state;
}

int WindowManager::move_to_top(size_t index) {
	WindowNode *window_index = first_node->next;

	while(window_index != NULL && window_index->window->index != index) {
		window_index = window_index->next;
	}

	if(window_index->next == NULL) return window_index->window->index;

	window_index->prev->next = window_index->next;
	
	if(window_index->next != NULL) {
		window_index->next->prev = window_index->prev;
	}

	WindowNode *new_last_window = window_index;

	//Go to last in list
	while(window_index->next != NULL) {
		window_index = window_index->next;
	}

	new_last_window->next = NULL;
	new_last_window->prev = window_index;
	window_index->next = new_last_window;

	reset_indices();

	return new_last_window->window->index;
}

void WindowManager::delete_window(size_t index) {
	WindowNode *window_index = first_node->next;

	window_count--;

	while(window_index != NULL && window_index->window->index != index) {
		window_index = window_index->next;
	}

	if(window_index == NULL) return;
	if(window_index->window->index != index) return;

	window_index->prev->next = window_index->next;

	if(window_index->next != NULL) {
		window_index->next->prev = window_index->prev;
	}

	window_index->window->delete_window();
	free(window_index);

	reset_indices();
}

void WindowManager::reset_indices() {
	WindowNode *window_index = first_node;

	size_t index = 1;

	while(window_index != NULL) {
		window_index->window->index = index;
		index++;
		window_index = window_index->next;
	}
}