#pragma once

#include <glm/glm.hpp>

using namespace glm;


dvec3 vec4Tovec3(dvec4 v){
	return dvec3(v.x,v.y,v.z);
}
