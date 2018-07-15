#include "Primitive.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <math.h>
#include "ColInfo.hpp"

using namespace std;
using namespace glm;

double EPSILON = 0.00001;

Primitive::~Primitive()
{
}

Sphere::~Sphere()
{
}


Sphere::Sphere()
{
	m_primitive = new NonhierSphere({0, 0, 0}, 1.0);
	skipCheck = true;
	maxRadius = m_primitive->maxRadius;
}

bool Sphere::hit(Ray &r, ColInfo& info)
{
	return m_primitive->hit(r, info);
}


Cube::Cube()
{
	m_primitive = new NonhierBox({0, 0, 0}, 1.0);
	skipCheck = true;
	maxRadius = m_primitive->maxRadius;
}

Cube::~Cube()
{
}

bool Cube::hit(Ray &r, ColInfo& info)
{
	return m_primitive->hit(r, info);
}

NonhierSphere::~NonhierSphere()
{
}

NonhierBox::~NonhierBox()
{
}

bool NonhierSphere::hit(Ray &r, ColInfo& info)
{

	dvec4 c(m_pos.x, m_pos.y, m_pos.z, 1);
	dvec4 o_c = r.origin - c;

	double A = dot(r.head,r.head);
	double B = 2 * dot(r.head, o_c);
	double C = glm::dot(o_c, o_c) - m_radius * m_radius;

	double indicator = B * B - 4 * A * C;

	if(indicator >= EPSILON){
		auto root1 = (-B - sqrt(indicator))/(2 * A);
		auto root2 = (-B + sqrt(indicator))/(2 * A);
		if(root1 < 0 && root2 < 0){
			return false;
		}
		if(root1 < 0) info.t = root2;
		else if(root2 < 0) info.t = root1;
		else info.t = glm::min(root1, root2);
		
		dvec4 intersection = r.origin + info.t * r.head;
		info.N = normalize(intersection - c);
		info.uv = dvec2(info.N.x/2.0 + 0.5,info.N.y/2.0 + 0.5);
		return true;
	}else {
		return false;
	}
}

void swap(double &d1, double &d2){
	double temp = d1;
	d1 = d2;
	d2 = temp;
}

// Adapted from <Realtime Collision Detection> by C. Ericson, Page 181
bool NonhierBox::hit(Ray &r, ColInfo& info)
{
	double tmin = 0.0;
	double tmax = INFINITY;
	dvec4 p = r.origin;
	dvec4 d = r.head;
	dvec3 amin = dvec3(m_pos);
	dvec3 amax = dvec3(m_pos) + dvec3(m_size);
	if(glm::abs(d.x) < EPSILON) {
		if(p.x < amin.x || p.x > amax.x) return false;
	}else{
		double ood = 1.0/d.x;
		double t1 = (amin.x - p.x) * ood;
		double t2 = (amax.x - p.x) * ood;
		if(t1 > t2) swap(t1, t2);
		if(t1>tmin) tmin = t1;
		if(t2<tmax) tmax = t2;
		if(tmin>tmax) return false;
	}
	if(glm::abs(d.y) < EPSILON) {
		if(p.y < amin.y || p.y > amax.y) return false;
	}else{
		double ood = 1.0/d.y;
		double t1 = (amin.y - p.y) * ood;
		double t2 = (amax.y - p.y) * ood;
		if(t1 > t2) swap(t1, t2);
		if(t1>tmin) tmin = t1;
		if(t2<tmax) tmax = t2;
		if(tmin>tmax) return false;
	}
	if(glm::abs(d.z) < EPSILON) {
		if(p.z < amin.z || p.z > amax.z) return false;
	}else{
		double ood = 1.0/d.z;
		double t1 = (amin.z - p.z) * ood;
		double t2 = (amax.z - p.z) * ood;
		if(t1 > t2) swap(t1, t2);
		if(t1>tmin) tmin = t1;
		if(t2<tmax) tmax = t2;
		if(tmin>tmax) return false;
	}
	info.t = tmin;
	auto intersection = p + info.t * d;
	auto diffmin = intersection - dvec4(amin, 1.0);
	auto diffmax = intersection - dvec4(amax, 1.0);
	if(glm::abs(diffmin.x) < EPSILON) {
		info.N = dvec4(-1.0,0.0,0,0);	
	}
	if(glm::abs(diffmax.x) < EPSILON) {
		info.N = dvec4(1.0,0.0,0,0);	
	}
	if(glm::abs(diffmin.y) < EPSILON) {
		info.N = dvec4(0.0,-1.0,0,0);	
	}
	if(glm::abs(diffmax.y) < EPSILON) {
		info.N = dvec4(0.0,1.0,0,0);	
	}
	if(glm::abs(diffmin.z) < EPSILON) {
		info.N = dvec4(0.0,0.0,-1.0,0);	
	}
	if(glm::abs(diffmax.z) < EPSILON) {
		info.N = dvec4(0.0,0.0,1.0,0);	
	}
	
	return true;
}

bool Primitive::hit(Ray &r, ColInfo& info)
{
	return false;
}