#pragma once

#include <stdint.h>

class Point {
	public:
	int64_t x, y;
	Point(int x, int y);
	bool operator!=(Point other_point);
};