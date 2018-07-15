#include <glm/ext.hpp>
#include <iostream>

#include "A4.hpp"
#include "Primitive.hpp"
#include "PhongMaterial.hpp"

using namespace std;
using namespace glm;

glm::dmat4 device2WorldMat;

extern int earlyReturn;
bool pp = false;
extern bool SHORTCUT;

double AMBIENCE = 0.5;

void A4_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {

  // Fill in raytracing code here...

	// Cache device 2 world transformation
	double d = 10.0f;
	double w = 2 * d * tan(radians(fovy/2));
    double h = 2 * d * tan(radians(fovy/2));
    double nx = image.width();
    double ny = image.height();
	{
		dmat4 T1 = translate(dmat4(), dvec3(-nx/2, -ny/2, d));
		dmat4 S2 = scale(dmat4(), glm::dvec3(-h/ny, w/nx, 1));

		vec3 w = normalize(view);
		vec3 u = cross(normalize(up), w);
		vec3 v = cross(u, w);
		double data[16] = {
			u.x, v.x, w.x, 0,
			u.y, v.y, w.y, 0,
			u.z, v.z, w.z, 0,
			0,   0,   0,   1
		};
		dmat4 R3 = transpose(make_mat4(data));

		dmat4 T4 = translate(dmat4(), dvec3(eye.x,eye.y,eye.z));

		device2WorldMat = T4 * R3 * S2 * T1;
	}

	std::cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
	      "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
	      "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
	      "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t he = image.height();
	size_t wi = image.width();

	unsigned int total = he * wi;
	unsigned int progress = 0;

	Ray r = {
		dvec4(eye, 1),
		device2World(30, 30) - dvec4(eye, 1)
	};
	cout<<to_string(rayColor(r, lights, 5, root, dvec3(
				glm::pow(1-30/float(he),2),
				glm::pow(1-30/float(he),2),
				cos(2*30/float(wi))
			)/2.0))<<endl;

	for (uint y = 0; y < he; ++y) {
		for (uint x = 0; x < wi; ++x) {
			if(x == 172 && y == 101) pp =true;
			Ray r = {
				dvec4(eye, 1),
				device2World(x, y) - dvec4(eye, 1)
			};
			auto color = rayColor(r, lights, 2, root, dvec3(
				glm::pow(1-y/float(he),2),
				glm::pow(1-y/float(he),2),
				cos(2*x/float(wi))
			)/2.0);
			image(x, y, 0) = color.x;
			image(x, y, 1) = color.y;
			image(x, y, 2) = color.z;
			progress ++;
			if(progress % (total/20) == 0){
				int prog = progress / ((total-1)/20);
				cout<<"progress : " <<prog*5 << "/100"<<endl;
			}
		}
	}
	

	cout<<"early returns: " <<earlyReturn<<endl;
}

bool notCloseTo0(glm::vec3 v)
{
	return glm::abs(v.x - 0.0) > EPSILON ||
		glm::abs(v.y - 0.0) > EPSILON ||
		glm::abs(v.z - 0.0) > EPSILON;
}

glm::dvec3 directLight(Ray& ir, glm::dvec4 P, glm::dvec4 N, const std::list<Light *> & lights, SceneNode* root, PhongMaterial material, int bounces){
	dvec3 col;
	dvec3 ks = material.m_ks;
	dvec3 kd = material.m_kd;
	double shininess = material.m_shininess;

	for (Light *l : lights) {
		Ray r = {
			P + normalize(dvec4(l->position,1) - P) * 0.01 ,
			dvec4(l->position,1) - P
		};
		r.origin.w = 1;
		r.head.w = 0;
		PhongMaterial mMaterial(vec3(),vec3(),0);

		double t;
		dvec4 mN;

		if(root->hit(r, t, mN, mMaterial, SHORTCUT) && t > EPSILON){
			continue;
		}
		if(notCloseTo0(kd)){
			double cosTheta = glm::max(dot(normalize(dvec4(l->position,1) - P), N), 0.0);
			double dist = length(r.head);

			dvec3 diffuse_light = (cosTheta * l->colour / (l->falloff[0] + dist * l->falloff[1] + dist * dist * l->falloff[2]));
			col+= kd * diffuse_light;
		}
		if(notCloseTo0(material.m_ks)){
			dvec4 reflected = normalize(ir.head) - 2.0 * dot(normalize(ir.head), N) * N;
			double cosTheta = glm::max(dot(normalize(dvec4(l->position,1) - P), reflected), 0.0);
			col += material.m_ks * glm::pow(cosTheta, shininess);
		}
	}

	if(notCloseTo0(material.m_ks)){
		dvec4 reflected = normalize(ir.head) - 2.0 * dot(normalize(ir.head), N) * N;
		Ray reflectedRay = {
			P + normalize(reflected) * 0.01,
			reflected
		};
		
		col += material.m_ks * rayColor(reflectedRay, lights, bounces-1, root, dvec3(00,0,0));
	}
	
	return col;
}

glm::vec3 rayColor(Ray & r, const std::list<Light *> & lights, int bounces, SceneNode* root, dvec3 background) {
	dvec3 kd;
	dvec3 ks;
	dvec3 ke;
	dvec3 col;
	double t;
	double shininess;
	dvec4 P;
	dvec4 N;

	PhongMaterial material(vec3(),vec3(),0);

	if(!bounces) return background;

	if(root->hit(r, t, N, material, SHORTCUT)) {
		col = ke + kd * AMBIENCE;
		P = r.origin + t * r.head;
		col += directLight(r, P, N, lights, root, material, bounces);
		return col;
	}else {
		return background;
	}

} 

glm::dvec4 device2World(int x, int y) { return device2World(dvec4(x, y, 0, 1)); }
glm::dvec4 device2World(dvec4 deviceCoord) { return device2WorldMat * deviceCoord; }