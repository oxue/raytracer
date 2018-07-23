#pragma once

#include <glm/glm.hpp>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"

#include "Ray.hpp"

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
);
float frand();
Ray getRefractedRay(Ray inRay, dvec4 N, dvec4 P);
glm::dvec4 device2World(glm::dvec4 deviceCoord);
glm::dvec4 device2World(double x, double y);
glm::vec3 rayColor(Ray & r, const std::list<Light *> & lights, int bounces, SceneNode* root, glm::dvec3 background, float t);
std::vector<dvec3> getPerturbed(dvec3 direction, double shininess);
glm::dvec3 directLight(
	Ray& ir,
	glm::dvec4 P,
	glm::dvec4 N,
	const std::list<Light *> & lights,
	SceneNode* root,
	glm::dvec3 kd,
	glm::dvec3 ks,
	double shinines,
	double refInd);

bool notCloseTo0(glm::vec3 v);
//bool hit(Ray &r, double &t, glm::dvec3 &kd, glm::dvec3 &ks, glm::dvec3 &ke);