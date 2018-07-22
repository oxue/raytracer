#include "Texture.hpp"
#include <string>
#include "lodepng.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>

using namespace std;
using namespace glm;

Texture::Texture(const std::string & path, bool isTransparent){
	transparent = isTransparent;
	auto error = lodepng::decode(*((vector<unsigned char>*)&data), width, height, path);
	if(!error){
		cout<<"Texture " << path << " loaded"<<endl;
		cout<<"width : "<< width<<endl;
		cout<<"height : "<< height<<endl;
		cout<<"length : "<< data.size()<<endl;
	}
	else {
		cout<<"there was an error loading image"<<lodepng_error_text(error)<<endl;	
	}
}

vec4 Texture::getColor(double u, double v)
{
	int index = int(u * width) + int(v * height) * width;
	rgba color = data[index];
	if(transparent) {
		vec4 ret = vec4(color.r/255.0,color.g/255.0,color.b/255.0, color.a/255.0);
		return ret;
	}
	return vec4(color.r/255.0,color.g/255.0,color.b/255.0, 1.0);
}