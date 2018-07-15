#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
using namespace std;

struct Ray{
	glm::dvec4 origin;
	glm::dvec4 head;

	void print() {
		cout<<"origin: "<<glm::to_string(origin)<<endl;
		cout<<"head: "<<glm::to_string(head)<<endl;
	}
};