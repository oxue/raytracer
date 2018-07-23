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

Cyl::~Cyl()
{
}


Cyl::Cyl()
{
	
}

Cone::~Cone()
{
}


Cone::Cone()
{
	
}



bool Cone::hit(Ray &r, ColInfo& info)
{

	double cosa = .95;
	double hh = 2;
	dvec3 cc(0,1,0);
	dvec3 v(0,-1,0);

	dvec3 ro(r.origin.x, r.origin.y, r.origin.z);
	dvec3 rd = vec3(r.head.x, r.head.y, r.head.z);
	double lenrd = length(rd);
	rd = rd/lenrd;

    dvec3 co = ro - cc;

    double a = dot(rd,v)*dot(rd,v) - cosa*cosa;
    double b = 2. * (dot(rd,v)*dot(co,v) - dot(rd,co)*cosa*cosa);
    double c = dot(co,v)*dot(co,v) - dot(co,co)*cosa*cosa;

    double det = b*b - 4.*a*c;
    if (det < 0.) return false;

    det = sqrt(det);
    double t1 = (-b - det) / (2. * a);
    double t2 = (-b + det) / (2. * a);

    // This is a bit messy; there ought to be a more elegant solution.
    double t = t1;
    if (t < 0. || t2 > 0. && t2 < t) t = t2;
    if (t < 0.) return false;

    dvec3 cp = ro + t*rd - cc;
    double h = dot(cp, v);
    if (h < 0. || h > hh) return false;

    dvec3 n = normalize(cp * dot(v, cp) / dot(cp, cp) - v);

    info.N = dvec4(n, 0);
    info.t = t/lenrd;
    return true;
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

dvec3 Cyl::computeNormal(dvec3 p, dvec3 q, Ray ray, double t)
{
	dvec4 Pray = ray.origin + ray.head * t;
	dvec3 P = dvec3(Pray.x, Pray.y, Pray.z);

	dvec3 base2P = P - p;

	dvec3 axis = q - p;
	double sproj = dot(base2P, normalize(axis));

	//cout<<sproj<<endl;
	if(abs(sproj - 1) < EPSILON) {
		
		return normalize(axis);
	}
	if(abs(sproj) < EPSILON) return normalize(-axis);

	return normalize(base2P - sproj * normalize(axis));
}

bool Cyl::hit(Ray &ray, ColInfo &info)
{
	dvec3 p(0);
	dvec3 q(0, 1, 0);
	double r = 0.5;
	dvec3 sa = dvec3(ray.origin.x, ray.origin.y, ray.origin.z);
	dvec3 sb =  20.0 * dvec3(ray.head.x, ray.head.y, ray.head.z) + sa;
	dvec3 d = q - p;
	dvec3 m = sa - p;
	dvec3 n = sb - sa;
	double md = dot(m, d);
	double nd = dot(n, d);
	double dd = dot(d, d);
	double &t = info.t;

	if (md < 0.0f && md + nd < 0.0f) return false;
	if (md > dd && md + nd > dd) return false;

	double nn = dot(n, n);
	double mn = dot(m, n);
	double a = dd * nn - nd * nd;
	double k = dot(m, m) - r * r;
	double c = dd * k - md * md;

	if(abs(a) < EPSILON) {
		if(c>0.0) return 0;
		if (md < 0.0f) {
			t = -mn / nn;  // Intersect segment against ’p’ endcap
		}
		else if (md > dd) {
			t = (nd - mn) / nn; // Intersect segment against ’q’ endcap
			cout<<'q'<<endl;
		}
		else t = 0.0f;

		info.t *= 20.0;
		info.N = dvec4(computeNormal(p, q, ray, t), 0);
		return true;
	}

	double b = dd * mn - nd * md;
	double discr = b * b - a * c;
	if (discr < 0.0f) return false; // No real roots; no intersection

	double t0 = t = (-b - sqrt(discr)) / a;

	if (md + t * nd < 0.0f) {
		if (nd <= 0.0f) return false;
		t = -md / nd;
		bool hit = k + t * (2 * mn + t * nn) <= 0.0f;
		info.t *= 20.0;
		info.N = dvec4(computeNormal(p, q, ray, t), 0);
		return hit;	
	} else if (md + t * nd > dd) {
		if (nd >= 0.0f) return false; 
		t = (dd - md) / nd;
		bool hit = k + dd - 2 * md + t * (2 * (mn - nd) + t * nn) <= 0.0f;
		info.t *= 20.0;
		info.N = dvec4(computeNormal(p, q, ray, t), 0);
		return hit;
	}
	t = t0;
	info.t *= 20.0;
	info.N = dvec4(computeNormal(p, q, ray, t), 0);
	return true;
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