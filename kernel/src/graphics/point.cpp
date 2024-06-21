#include "point.hpp"

Point::Point(int x, int y) {
	this->x = x;
	this->y = y;
}

bool Point::operator!=(Point other_point) {
	if(this->x == other_point.x && this->y == other_point.y) return false;
	return true;
}