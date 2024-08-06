#include "cube.hpp"

Cube::Cube(Canvas *canvas, vec3 pos, vec3 size) : pos(pos), size(size) {\
	this->canvas = canvas;

	points[0] = {pos.x,          pos.y,          pos.z};
	points[1] = {pos.x + size.x, pos.y,          pos.z};
	points[2] = {pos.x + size.x, pos.y + size.y, pos.z};
	points[3] = {pos.x,          pos.y + size.y, pos.z};

	points[4] = {pos.x,          pos.y,          pos.z + size.z};
	points[5] = {pos.x + size.x, pos.y,          pos.z + size.z};
	points[6] = {pos.x + size.x, pos.y + size.y, pos.z + size.z};
	points[7] = {pos.x,          pos.y + size.y, pos.z + size.z};

	for(vec3 point : points) {
		centroid.x += point.x;
		centroid.y += point.y;
		centroid.z += point.z;
	}
	centroid.x /= 8;
	centroid.y /= 8;
	centroid.z /= 8;
}

void Cube::render() {
	for(connection con : cube_connections) {
		if(canvas == NULL) break;
		canvas->gfx->draw_line(Point(points[con.a].x, points[con.a].y), Point(points[con.b].x, points[con.b].y));
		//canvas->gfx->draw_line(Point(1,1), Point(2,2));
	}
}