#pragma once

#include "../graphics.hpp"
#include "../../shell/window_mgr/canvas.hpp"

using namespace Graphics3D;

static connection cube_connections[] = {
	{0, 4},
	{1, 5},
	{2, 6},
	{3, 7},

	{0, 1},
	{1, 2},
	{2, 3},
	{3, 0},

	{4, 5},
	{5, 6},
	{6, 7},
	{7, 4}
};

class Cube {
	private:
	vec3 points[8];
	vec3 centroid;
	Canvas *canvas;

	public:
	vec3 pos;
	vec3 size;

	Cube(Canvas *canvas, vec3 pos, vec3 size);
	void render();
};