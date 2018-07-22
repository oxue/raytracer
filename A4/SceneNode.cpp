#include "SceneNode.hpp"

#include "cs488-framework/MathUtils.hpp"
#include "Ray.hpp"
#include <math.h>
#include "Primitive.hpp"
#include "PhongMaterial.hpp"
#include "ColInfo.hpp"

#include <iostream>
#include <sstream>
using namespace std;

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

extern int earlyReturn;

// Static class variable
unsigned int SceneNode::nodeInstanceCount = 0;


//---------------------------------------------------------------------------------------
SceneNode::SceneNode(const std::string& name)
  : m_name(name),
	m_nodeType(NodeType::SceneNode),
	trans(mat4()),
	invtrans(mat4()),
	m_nodeId(nodeInstanceCount++)
{

}

//---------------------------------------------------------------------------------------
// Deep copy
SceneNode::SceneNode(const SceneNode & other)
	: m_nodeType(other.m_nodeType),
	  m_name(other.m_name),
	  trans(other.trans),
	  invtrans(other.invtrans)
{
	for(SceneNode * child : other.children) {
		this->children.push_front(new SceneNode(*child));
	}
}

//---------------------------------------------------------------------------------------
SceneNode::~SceneNode() {
	for(SceneNode * child : children) {
		delete child;
	}
}

//---------------------------------------------------------------------------------------
void SceneNode::set_transform(const glm::mat4& m) {
	trans = m;
	invtrans = glm::inverse(m);
}

//---------------------------------------------------------------------------------------
const glm::mat4 SceneNode::get_transform(float timeStep) const {
	if(timeStep != 0){
		return get_timeTransform(timeStep) * trans;
	}
	return trans;
}

const glm::mat4 SceneNode::get_timeTransform(float timeStep) const {
	return glm::translate(0.5 * timeStep* timeStep * m_acceleration + timeStep * m_velocity) * 
		glm::rotate(m_rotational_velocity.x * timeStep, vec3(0,0,1));
}

//---------------------------------------------------------------------------------------
const glm::mat4 SceneNode::get_inverse(float timeStep) const {
	if(timeStep != 0) {
		return glm::inverse(get_timeTransform(timeStep) * trans);
	}
	return invtrans;
}

//---------------------------------------------------------------------------------------
void SceneNode::add_child(SceneNode* child) {
	children.push_back(child);
}

double SceneNode::getMaxRadius(float t) {
	return 10000;
	if(cachedMaxR >= 0) return cachedMaxR;

	double maxR = -1.0;
	for(SceneNode* node:children){
		double dist = glm::length(get_transform(t) * dvec4(0,0,0,1.0)) 
		+ glm::length(get_transform(t) * dvec4(node->getMaxRadius(t),0,0,1.0));
		if(dist > maxR)
			maxR = dist;
	}

	cachedMaxR = maxR;
	return maxR;
}

//---------------------------------------------------------------------------------------
void SceneNode::remove_child(SceneNode* child) {
	children.remove(child);
}

//---------------------------------------------------------------------------------------
void SceneNode::rotate(char axis, float angle) {
	vec3 rot_axis;

	switch (axis) {
		case 'x':
			rot_axis = vec3(1,0,0);
			break;
		case 'y':
			rot_axis = vec3(0,1,0);
	        break;
		case 'z':
			rot_axis = vec3(0,0,1);
	        break;
		default:
			break;
	}
	mat4 rot_matrix = glm::rotate(degreesToRadians(angle), rot_axis);
	set_transform( rot_matrix * trans );
}

//---------------------------------------------------------------------------------------
void SceneNode::scale(const glm::vec3 & amount) {
	set_transform( glm::scale(amount) * trans );
}

//---------------------------------------------------------------------------------------
void SceneNode::translate(const glm::vec3& amount) {
	set_transform( glm::translate(amount) * trans );
}

void SceneNode::setVelocity(const glm::vec3& amount) {
	m_velocity = amount;
}

void SceneNode::setRotationalVelocity(const glm::vec3& amount) {
	m_rotational_velocity = amount;
}

void SceneNode::setAcceleration(const glm::vec3& amount) {
	m_acceleration = amount;
}

//---------------------------------------------------------------------------------------
int SceneNode::totalSceneNodes() const {
	return nodeInstanceCount;
}

dvec4 SceneNode::backTraceNormal(dvec4 N, float t){
	return normalize(vec4(transpose(dmat3(get_inverse(t))) * dvec3(N),0.0));
}

bool SceneNode::hit(Ray &r, ColInfo &info, bool shortcut, float t) {
	info.t = INFINITY;
	ColInfo mInfo;
	mInfo.t = INFINITY;

	bool hit = false;

	for(SceneNode* child : children) {
		Ray deepRay = {
			dmat4(get_inverse(t)) * r.origin,
			dmat4(get_inverse(t)) * r.head
		};

		if(child->hit(deepRay, mInfo, shortcut, t)){

			hit = true;
			if(mInfo.t < info.t){
				info = mInfo;
			}
		}
	}
	info.N = backTraceNormal(info.N, t);
	return hit;
}

//---------------------------------------------------------------------------------------
std::ostream & operator << (std::ostream & os, const SceneNode & node) {

	//os << "SceneNode:[NodeType: ___, name: ____, id: ____, isSelected: ____, transform: ____"
	switch (node.m_nodeType) {
		case NodeType::SceneNode:
			os << "SceneNode";
			break;
		case NodeType::GeometryNode:
			os << "GeometryNode";
			break;
		case NodeType::JointNode:
			os << "JointNode";
			break;
	}
	os << ":[";

	os << "name:" << node.m_name << ", ";
	os << "id:" << node.m_nodeId;

	os << "]\n";
	return os;
}
