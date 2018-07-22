#pragma once

#include <glm/glm.hpp>
#include "PhongMaterial.hpp"
#include <iostream>

using namespace glm;
using namespace std;

struct ColInfo {
	// UV Coordinates at the collision
	dvec2 uv;
	// Normal at the Collision site
	dvec4 N;
	// t Parameter for ray at Collision
	double t;
	// material params
	PhongMaterial material;

	// Tangent and Bitangent
	dvec3 tangent;
	dvec3 bitangent;

	ColInfo():
	material(vec3(),vec3(),0.0f),
	uv(0.0),
	N(0.0),
	t(0.0),
	tangent(0.0),
	bitangent(0.0)
	{
		
	}

	void print() {
		cout<<"colinfo {"<<endl;
		cout<<to_string(uv)<<endl;
		cout<<to_string(N)<<endl;
		cout<<t<<endl;
		cout<<"}"<<endl;
	}
};