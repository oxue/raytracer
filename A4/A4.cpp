#include <glm/ext.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <mutex>
#include <map>
#include <utility>

#include "A4.hpp"
#include "Primitive.hpp"
#include "PhongMaterial.hpp"
#include "SceneNode.hpp"
#include "GeometryNode.hpp"

#include <cstdio>
#include <ctime>

#define ENABLE_GLOSSY 0
#define ENABLE_REFRACTION 0
#define ENABLE_SOFT_SHADOWS 0
#define ENABLE_MOTION_BLUR 0
#define ENABLE_DEPTH_OF_FIELD 0
#define ENABLE_AA 0
#define ENABLE_GRID 0

#define BOUNCES 5
#define NUM_THREADS 2
#define PROGRESS_FLUSH_RATE 1000
#define TIMESTEPS 20
#define DOFSAMPLES 10
#define AMBIENCE 0.2f
#define DOF_AMOUNT 2.0f
#define FOCAL_LENGTH 1

#define uint uint32_t

using namespace std;
using namespace glm;

extern int earlyReturn;
extern bool SHORTCUT;

dmat4        device2WorldMat;
mutex        progressMutex;
uint         progress = 0;

dvec3 background(0.9, 0.95, 0.9);

double fresnel(glm::dvec4 normal,
							  const glm::dvec4 & invec,
							  double n1,
							  double n2)
{
	if(dot(invec, normal) > 0) {
		swap(n1, n2);
		normal = -normal;
	}

	// Test for TIR.
	double n = n1 / n2;
	double cosI = -glm::dot(invec, normal);
	double sinT_2 = n * n * (1 - cosI * cosI);
	
	if (sinT_2 > 1) {
		// TIR
		return 1.0;
	}
	
	double cosT = sqrt(1.0 - sinT_2);
	
	double Rs = (n2 * cosI - n1 * cosT) / (n2 * cosI + n1 * cosT);
	Rs = Rs * Rs;
	
	double Rp = (n2 * cosT - n1 * cosI) / (n1 * cosI + n2 * cosT);
	Rp = Rp * Rp;
	
	return (Rs + Rp) / 2.0;
}

Ray getPrimaryRay(vec3 eye, double x, double y){
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

double averageDifferential(Image &image, int x, int y) {
	dvec3 p1(image(x, y, 0), image(x, y, 1), image(x, y, 3));
	dvec3 p2(image(x+1, y, 0), image(x+1, y, 1), image(x+1, y, 3));
	dvec3 p3(image(x, y+1, 0), image(x, y+1, 1), image(x, y+1, 3));
	dvec3 p4(image(x+1, y+1, 0), image(x+1, y+1, 1), image(x+1, y+1, 3));

	double total = 0;
	total += length(p1 - p2);
	total += length(p1 - p3);
	total += length(p1 - p4);

	total += length(p2 - p3);
	total += length(p2 - p4);

	total += length(p3 - p4);

	return total / 6.0;

}

void index1Guy(map<string, vector<GeometryNode*>> &grid, int X, int Y, int Z, GeometryNode * node){
	string key = std::to_string(X) + "/" + std::to_string(Y) + "/" + std::to_string(Z);
	if(grid.count(key) == 0) {
		vector<GeometryNode*> list;
		grid[key] = list;
	}
	grid[key].push_back(node);
}

void indexObjects(map<string, vector<GeometryNode*>> &grid, SceneNode * root)
{
	for(SceneNode* child:root->children) {
		indexObjects(grid, child);	
	}

	if(root->m_nodeType == NodeType::GeometryNode) {
		GeometryNode* m_node = static_cast <GeometryNode*> (root);
		dvec4 pos = root->get_transform(0) * dvec4(0,0,0,1);
		int X1 = floor(pos.x - 0.1);
		int X2 = floor(pos.x + 0.1);

		int Y1 = floor(pos.y - 0.1);
		int Y2 = floor(pos.y + 0.1);

		int Z1 = floor(pos.z - 0.1);
		int Z2 = floor(pos.z + 0.1);

		index1Guy(grid, X1, Y1, Z1, m_node);
		index1Guy(grid, X2, Y1, Z1, m_node);
		index1Guy(grid, X1, Y2, Z1, m_node);
		index1Guy(grid, X1, Y1, Z2, m_node);

		index1Guy(grid, X2, Y2, Z1, m_node);
		index1Guy(grid, X1, Y2, Z2, m_node);
		index1Guy(grid, X2, Y1, Z2, m_node);
		index1Guy(grid, X2, Y2, Z2, m_node);

		return;
	}
}

map<string, vector<GeometryNode*>> m_grid;

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
	
	if(ENABLE_GRID) {
		indexObjects(m_grid, root);
	}

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

	// adaptive super sampling pass
	if(!ENABLE_AA) return;

	map<int, pair<int, int>> ssLocations;
	for (uint y = 0; y < image.height()-1; ++y) {
		for (uint x = 0; x < image.width()-1; ++x) {
			double delta = averageDifferential(image, x, y);
			if(delta > 0.3){
				ssLocations[y * image.height() + x] = make_pair(x, y);
				ssLocations[y * image.height() + (x+1)] = make_pair((x+1), y);
				ssLocations[(y+1) * image.height() + (x+1)] = make_pair((x+1), (y+1));
				ssLocations[(y+1) * image.height() + x] = make_pair(x, (y+1));
			}
		}
	}

	for(auto entry:ssLocations) {
		int x = entry.second.first;
		int y = entry.second.second;

		auto r1 = getPrimaryRay(eye, x + (0.4 + frand() * 0.2), y + (0.4 + frand() * 0.2));
		auto r2 = getPrimaryRay(eye, x + (0.4 + frand() * 0.2), y - (0.4 + frand() * 0.2));
		auto r3 = getPrimaryRay(eye, x - (0.4 + frand() * 0.2), y + (0.4 + frand() * 0.2));
		auto r4 = getPrimaryRay(eye, x - (0.4 + frand() * 0.2), y - (0.4 + frand() * 0.2));

		auto c0 = vec3(image(x, y, 0), image(x, y, 1), image(x, y, 2));
		vec3 c1 = rayColor(r1, lights, BOUNCES, root, background, 0);
		vec3 c2 = rayColor(r2, lights, BOUNCES, root, background, 0);
		vec3 c3 = rayColor(r3, lights, BOUNCES, root, background, 0);
		vec3 c4 = rayColor(r4, lights, BOUNCES, root, background, 0);

		auto newColor = (c1 + c2 + c3 + c4 + c0) / 5.0;

 		image(x, y, 0) = newColor.x;
		image(x, y, 1) = newColor.y;
		image(x, y, 2) = newColor.z;
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
			if(blocked && info.t > EPSILON){
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

	double transmittance = 1- fresnel(N, normalize(ir.head), 1, 1.3);
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

bool gridHit(Ray & r, ColInfo &info, map<string, vector<GeometryNode*>> grid)
{
	dvec4 sa = r.origin;
	dvec4 sb = r.origin + 20.0 * r.head;

	int X0 = floor(sa.x);
	int Y0 = floor(sa.y);
	int Z0 = floor(sa.y);

	int X1 = floor(sb.x);
	int Y1 = floor(sb.y);
	int Z1 = floor(sb.z);

	int ix = (X1 > X0) ? 1 : ((X1 < X0) ? -1 : 0);
	int iy = (Y1 > Y0) ? 1 : ((Y1 < Y0) ? -1 : 0);
	int iz = (Z1 > Z0) ? 1 : ((Z1 < Z0) ? -1 : 0);
	
	int gx = X0;
    int gy = Y0;
    int gz = Z0;

    //pla
    int gxp = X0 + (X1 > X0 ? 1 : 0);
    int gyp = Y0 + (Y1 > Y0 ? 1 : 0);
    int gzp = Z0 + (Z1 > Z0 ? 1 : 0);

    // used for margins
    int vx = sb.x == sa.x ? 1 : sb.x - sa.x;
    int vy = sb.y == sa.y ? 1 : sb.y - sa.y;
    int vz = sb.z == sa.z ? 1 : sb.z - sa.z;

    //error
    int vxvy = vx * vy;
    int vxvz = vx * vz;
    int vyvz = vy * vz;

	int errx = (gxp - sa.x) * vyvz;
    int erry = (gyp - sa.y) * vxvz;
    int errz = (gzp - sa.z) * vxvy;

    int derrx = ix * vyvz;
    int derry = iy * vxvz;
    int derrz = iz * vxvy;

	while (abs(gx) < 4 && abs(gz) < 4 && abs(gy) < 4) {
		string key = std::to_string(gx) + "/" + std::to_string(gy) + "/" + std::to_string(gz);

		vector<GeometryNode*> elems = grid[key];
		info.t = INFINITY;
		ColInfo mInfo;
		mInfo.t = INFINITY;

		bool hit = false;
		for(GeometryNode *gnode : elems) {
			if(gnode->hit(r, mInfo, false, 0)){
				hit = true;
				if(mInfo.t < info.t){
					info = mInfo;
				}
			}
		}
		if(hit) return true;

		if (gx == X1 && gy == Y1 && gz == Z1) break;

		//e1 = e + 
		int xr = abs(errx);
        int yr = abs(erry);
        int zr = abs(errz);

        if (ix != 0 && (iy == 0 || xr < yr) && (iz == 0 || xr < zr)) {
            gx += ix;
            errx += derrx;
        }
        else if (iy != 0 && (iz == 0 || yr < zr)) {
            gy += iy;
            erry += derry;
        }
        else if (iz != 0) {
            gz += iz;
            errz += derrz;
        }
	}

	return false;
}

glm::vec3 rayColor(Ray & r, const std::list<Light *> & lights, int bounces, SceneNode* root, dvec3 background, float t) {
	dvec3 col;
	dvec4 P;
	ColInfo info;

	if(!bounces) return background;

	if(ENABLE_GRID) {
		if(gridHit(r, info, m_grid)){
			col = /*ke + */info.material.m_kd * AMBIENCE;
			P = r.origin + info.t * r.head;
			col += directLight(r, P, info.N, lights, root, info.material, bounces, 1.0, 0);
			return col;
		} else {
			return background;
		}
	}else if(root->hit(r, info, SHORTCUT, t)) {
		col = /*ke + */info.material.m_kd * AMBIENCE;
		P = r.origin + info.t * r.head;
		col += directLight(r, P, info.N, lights, root, info.material, bounces, 1.0, t);
		return col;
	}else {
		return background;
	}

} 

glm::dvec4 device2World(double x, double y) { return device2World(dvec4(x, y, 0, 1)); }
glm::dvec4 device2World(dvec4 deviceCoord) { return device2WorldMat * deviceCoord; }