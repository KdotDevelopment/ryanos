#pragma once

#include "../../graphics/point.hpp"

enum MouseState {
	M_RIGHT,
	M_MIDDLE,
	M_LEFT,
	M_NONE
};

struct Mouse {
	MouseState button_state;
	Point pos;
	Point last_pos;
	Point delta_pos;
};