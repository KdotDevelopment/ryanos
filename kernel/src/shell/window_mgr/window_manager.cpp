#include "window_manager.hpp"

#include "../../memory/mem.hpp"
#include "../../graphics/graphics.hpp"
#include "../out.hpp"

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
	id_count++;
	
	if(window_count > 1) { //Why four? I have no idea... however it seems to work :)
		while(window_index->next != NULL) {
			window_index = window_index->next;
		}

		if(window_index->prev == NULL) return NULL;

		new_node->next = NULL;
		new_node->prev = window_index;
		window_index->next = new_node;
		new_node->window = new Window(initial_pos, size, window_count);
		new_node->window->index = 1; //will be reset anyway
		new_node->window->id = id_count;
		window_index->next = new_node;

		reset_indices();
		return new_node->window;
	}

	while(window_index->next != NULL) {
		window_index = window_index->next;
	}

	window_index->next = new_node;
	new_node->next = NULL;
	new_node->prev->next = new_node;
	new_node->prev = window_index;
	new_node->window = new Window(initial_pos, size, window_count);
	new_node->window->index = 1; //will be reset anyway
	new_node->window->id = id_count;

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

		if(window_index->window->hover_window(mouse->last_pos) && window_index->window->index == focus_index) {
			window_index->window->set_relative_mouse_pos(mouse->last_pos);
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

Window *WindowManager::get_from_id(size_t id) {
	WindowNode *window_index = first_node->next;
	if(id < 1) return NULL;

	while(window_index != NULL) {
		if(window_index->window->id == id) break;
		if(window_index->next == NULL) break;
		window_index = window_index->next;
	}
	if(window_index->window->id == id) return window_index->window;
	else return NULL;
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
	free(window_index->window);
	window_index->window = NULL;
	window_index = NULL;

	reset_indices();
}

void WindowManager::reset_indices() {
	WindowNode *window_index = first_node;

	size_t index = 0;

	while(window_index != NULL) {
		window_index->window->index = index;
		if(index == window_count) {
			window_index->window->has_focus = true;
		}else {
			window_index->window->has_focus = false;
		}
		index++;
		window_index = window_index->next;
	}
}