#include "../graphics.hpp"
#include "../../lib/math.hpp"

void Graphics3D::rotate(vec3 *point, float x, float y, float z) {
	float rad = 0;

	rad = x;
	point->y = cos(rad) * point->y - sin(rad) * point->z;
	point->z = sin(rad) * point->y + cos(rad) * point->z;

	rad = y;
	point->x = cos(rad) * point->x + sin(rad) * point->z;
	point->z = -sin(rad) * point->x + cos(rad) * point->z;

	rad = z;
	point->x = cos(rad) * point->x - sin(rad) * point->y;
	point->y = sin(rad) * point->x + cos(rad) * point->y;
}