#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

using namespace glm;
using namespace std;

struct rgba{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

class Texture{
public:
	Texture(const std::string &path, bool isTransparent);

	vector<rgba> data;
	unsigned int width;
	unsigned int height;
	bool transparent;

	vec4 getColor(double u, double v);
};
