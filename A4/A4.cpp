#include <glm/ext.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <mutex>

#include "A4.hpp"
#include "Primitive.hpp"
#include "PhongMaterial.hpp"

#define ENABLE_GLOSSY 0
#define ENABLE_REFRACTION 1
#define ENABLE_SOFT_SHADOWS 0
#define ENABLE_MOTION_BLUR 0
#define ENABLE_DEPTH_OF_FIELD 0

#define BOUNCES 5
#define NUM_THREADS 2
#define PROGRESS_FLUSH_RATE 1000
#define TIMESTEPS 20
#define DOFSAMPLES 10
#define AMBIENCE 0.2f
#define DOF_AMOUNT 2.0f
#define FOCAL_LENGTH 4

#define uint uint32_t

using namespace std;
using namespace glm;

extern int earlyReturn;
extern bool SHORTCUT;

dmat4        device2WorldMat;
mutex        progressMutex;
uint         progress = 0;

dvec3 background(0.9, 0.95, 0.9);

double simplifiedFresnelModel(glm::dvec4 normal,
							  const glm::dvec4 & IncomingVector,
							  double fromReflectiveIndex,
							  double toReflectiveIndex)
{
	if(dot(IncomingVector, normal) > 0) {
		swap(fromReflectiveIndex, toReflectiveIndex);
		normal = -normal;
	}

	// Test for TIR.
	double n = fromReflectiveIndex / toReflectiveIndex;
	double cosI = -glm::dot(IncomingVector, normal);
	double sinT_2 = n * n * (1 - cosI * cosI);
	
	if (sinT_2 > 1) {
		// TIR
		return 1.0;
	}
	
	double cosT = sqrt(1.0 - sinT_2);
	
	double Rs = (toReflectiveIndex * cosI - fromReflectiveIndex * cosT) / (toReflectiveIndex * cosI + fromReflectiveIndex * cosT);
	Rs = Rs * Rs;
	
	double Rp = (toReflectiveIndex * cosT - fromReflectiveIndex * cosI) / (fromReflectiveIndex * cosI + toReflectiveIndex * cosT);
	Rp = Rp * Rp;
	
	return (Rs + Rp) / 2.0;
}

Ray getPrimaryRay(vec3 eye, uint x, uint y){
	dvec4 eyeVec = dvec4(eye, 1);
	Ray r = {
		eyeVec,
		device2World(x, y) - eyeVec
	};

	return r;
}

vector<Ray> getDOFRays(Ray r, vec3 focalPoint) {
	vector<Ray> ret;
	for(auto i = 0; i < DOFSAMPLES; i++){
		for(auto j = 0; j < DOFSAMPLES; j++){
			float di = DOFSAMPLES;
			di = i - di/2.0;
			float dj = DOFSAMPLES;
			dj = j - dj/2.0;
			if(di * di + dj * dj > DOFSAMPLES * DOFSAMPLES / 4.0f) continue;
			float randomRadius = frand();
			float randXPos = cos(frand() * 3.1415) * randomRadius;
			float randYPos = sqrt(randomRadius * randomRadius - randXPos * randXPos);
			di += randXPos;
			dj += randYPos;
			dmat4 randX = glm::rotate(di/DOFSAMPLES* 2 * DOF_AMOUNT * 3.1415/180.0, dvec3(0,1,0));
			dmat4 randY = glm::rotate(dj/DOFSAMPLES* 2 * DOF_AMOUNT * 3.1415/180.0, dvec3(1,0,0));
			Ray dofRay = {
				randX * randY * (r.origin - dvec4(focalPoint, 1)) + dvec4(focalPoint, 1),
				transpose(inverse(randX * randY)) * r.head
			};
			ret.push_back(dofRay);	
		}
		
	}
	return ret;
}

void tracerWorker(
	int offset, 
	Image& image,
	vec3 eye,
	std::list<Light *> lights,
	SceneNode* root)
{
	size_t he = image.height();
	size_t wi = image.width();

	int pointsProcessed = 0;

	for (uint y = 0; y < he; ++y) {
		for (uint x = 0; x < wi; ++x) {
			if ((y * he + x) % NUM_THREADS == offset){
				Ray r = getPrimaryRay(eye, x, y);

				vec3 color;

				if(ENABLE_MOTION_BLUR) {
					int samples = 0;
					for(auto t = 0; t < TIMESTEPS; t++){
						color += rayColor(r, lights, BOUNCES, root, background, (t + frand() - 0.5) * 0.5);
						samples += 1;
					}
					color = color / float(samples);
				} else if(ENABLE_DEPTH_OF_FIELD) {
					vector<Ray> DOFRays = getDOFRays(r, vec3(0,0,FOCAL_LENGTH));
					for (Ray dofr:DOFRays) {
						color += rayColor(dofr, lights, BOUNCES, root, background, 0);
					}
					color = color / DOFRays.size();
				} else {
					color = rayColor(r, lights, BOUNCES, root, background, 0);
				}

				

				image(x, y, 0) = color.x;
				image(x, y, 1) = color.y;
				image(x, y, 2) = color.z;

				pointsProcessed ++;
				if(pointsProcessed >= PROGRESS_FLUSH_RATE){
					progressMutex.lock();
					pointsProcessed = 0;
					progress += PROGRESS_FLUSH_RATE;
					float total = he * wi;
					cout << "progress report from thread " << offset << ": " << (int)(progress / total * 100) << endl;
					progressMutex.unlock();
				}
			}
		}
	}
}


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

	vector<thread> workers;
	for( int i = 0; i < NUM_THREADS; i++){
		workers.emplace_back(tracerWorker, i, ref(image), eye, lights, root);	
	}
	for (auto& t : workers){
		t.join();
	}

	cout<<"early returns: " <<earlyReturn<<endl;
}

bool notCloseTo0(glm::vec3 v)
{
	return glm::abs(v.x - 0.0) > EPSILON ||
		glm::abs(v.y - 0.0) > EPSILON ||
		glm::abs(v.z - 0.0) > EPSILON;
}

dvec3 getRandomVecInUnitSphere() {
	dvec3 randomVec(frand(),frand(),frand());
	while(dot(randomVec, randomVec) > 1){
		randomVec = dvec3(frand(),frand(),frand());
	}
	return randomVec;
}

vector<dvec3> getPerturbedLight(Light *l) {
	vector<dvec3> ret;
	for (auto i = 0; i < 4; i++){
		for (auto j = 0; j < 4; j++){
			for (auto k = 0; k < 4; k++){
				ret.push_back(dvec3(l->position) + l->radius * dvec3(i-2.0, j -2.0, k-2.0)/2.0);
			}
		}
	}
	return ret;
	for (auto i = 0; i < 100; i++){
		ret.push_back(dvec3(l->position) + l->radius * getRandomVecInUnitSphere());
	}
	return ret;
}

glm::dvec3 directLight(Ray& ir, glm::dvec4 P, glm::dvec4 N, const std::list<Light *> & lights, SceneNode* root, PhongMaterial material, int bounces, double refInd, float t){
	dvec3 col;

	// Shadow rays
	for (Light *l : lights) {
		vector<dvec3> lightPositions;
		if(ENABLE_SOFT_SHADOWS) {
			lightPositions = getPerturbedLight(l);
		}else{
			lightPositions.push_back(l->position);
		}

		dvec3 shadowRayTotal(0);

		for(auto lp:lightPositions) {
			Ray r = {
				P + normalize(dvec4(lp,1) - P) * 0.01,
				dvec4(lp,1) - P
			};
			r.origin.w = 1;
			r.head.w = 0;

			ColInfo info;

			float blockFactor = 1.0f;
			auto blocked = root->hit(r, info, true, t);

			if(blocked && info.t > EPSILON && info.material.m_kt < EPSILON){
				continue;
			}
			if(blocked){
				blockFactor = info.material.m_kt;
			}

			if(notCloseTo0(material.m_kd)){
				double cosTheta = glm::max(dot(normalize(dvec4(l->position,1) - P), N), 0.0);
				double dist = length(r.head);

				vec3 diffuse_light = (cosTheta * l->colour / (l->falloff[0] + dist * l->falloff[1] + dist * dist * l->falloff[2]));
				shadowRayTotal+= material.m_kd * diffuse_light * blockFactor;
			}
			if( material.m_shininess != 0){
				dvec4 reflected = normalize(ir.head) - 2.0 * dot(normalize(ir.head), N) * N;
				double cosTheta = glm::max(dot(normalize(dvec4(l->position,1) - P), reflected), 0.0);
				shadowRayTotal += material.m_ks * glm::pow(cosTheta, material.m_shininess) * blockFactor;
			}
		}

		shadowRayTotal = shadowRayTotal/lightPositions.size();
		col += shadowRayTotal;
		
	}

	double transmittance = 1- simplifiedFresnelModel(N, normalize(ir.head), 1, 1.3);
	transmittance = transmittance * transmittance;
	// Reflection
	if(material.m_kr > 0.001f){
		dvec4 reflected = normalize(ir.head) - 2.0 * dot(normalize(ir.head), N) * N;
		Ray reflectedRay = {
			P + normalize(reflected) * 0.01,
			reflected
		};

		if(ENABLE_GLOSSY){
			vec3 reflectedColor(0);
			auto perturbedRays = getPerturbed(vec3(reflected), material.m_shininess);
			for(auto dir:perturbedRays){
				Ray glossRay = {
					reflectedRay.origin,
					dvec4(dir,0)
				};
				reflectedColor += rayColor(glossRay, lights, bounces-1, root, background, t);
			}
			col += material.m_kr * reflectedColor / (double)perturbedRays.size();
		}else{

			col += (1-transmittance) * rayColor(reflectedRay, lights, bounces-1, root, background, t);
		}
		
	}
	if(ENABLE_REFRACTION){
		if(material.m_kt > 0.001f){
			Ray refractedRay = getRefractedRay(ir, N, P);
			col += transmittance * rayColor(refractedRay, lights, bounces-1, root, background, t);	
		}
	}
	
	return col;
}

Ray getRefractedRay(Ray inRay, dvec4 N, dvec4 P){
	double n1 = 1;
	double n2 = 1.3;
	if(dot(inRay.head, N) > 0) {
		swap(n1, n2);
		N = -N;
	}
	double n1overn2 = n1 / n2;
	double cosThetai = -dot(N, normalize(inRay.head));
	double sinThetai2 = 1 - cosThetai * cosThetai;
	double sinThetat2 = n1overn2 * n1overn2 * sinThetai2;
	if(sinThetat2 > 1) {
		// internal
	}
	dvec4 t = normalize(n1overn2 * normalize(inRay.head) + (n1overn2 * cosThetai - sqrt(1- sinThetat2)) * N);

	Ray refractedRay = {
		P + 0.01 * t,
		t
	};

	return refractedRay;
}

float frand() {
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	return r;
}

vector<dvec3> getPerturbed(dvec3 direction, double shininess){

	vector<dvec3> ret;
	int i = 10;
	while(i--){
		dvec3 randomVec = vec3(frand() - 0.5,frand() - 0.5,frand() - 0.5);
		dvec3 randomDir = normalize(cross(direction, randomVec));
		dvec3 perturbed = direction + randomDir * (double)frand() * 0.1;
		ret.push_back(perturbed);
	}
	return ret;
}

glm::vec3 rayColor(Ray & r, const std::list<Light *> & lights, int bounces, SceneNode* root, dvec3 background, float t) {
	dvec3 col;
	dvec4 P;
	ColInfo info;

	if(!bounces) return background;

	if(root->hit(r, info, SHORTCUT, t)) {
		col = /*ke + */info.material.m_kd * AMBIENCE;
		P = r.origin + info.t * r.head;
		col += directLight(r, P, info.N, lights, root, info.material, bounces, 1.0, t);
		return col;
	}else {
		return background;
	}

} 

glm::dvec4 device2World(int x, int y) { return device2World(dvec4(x, y, 0, 1)); }
glm::dvec4 device2World(dvec4 deviceCoord) { return device2WorldMat * deviceCoord; }