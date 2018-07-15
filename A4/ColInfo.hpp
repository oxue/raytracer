#pragma once

#include <glm/glm.hpp>
#include "PhongMaterial.hpp"

using namespace glm;

struct ColInfo {
	// UV Coordinates at the collision
	dvec2 uv;
	// Normal at the Collision site
	dvec4 N;
	// t Parameter for ray at Collision
	double t;
	// material params
	PhongMaterial material;

	ColInfo():
	material(vec3(),vec3(),0.0f)
	{
		
	}
};