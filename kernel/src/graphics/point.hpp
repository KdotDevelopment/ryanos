#pragma once

class Point {
	public:
	unsigned int x, y;
	Point(int x, int y);
	bool operator!=(Point other_point);
};