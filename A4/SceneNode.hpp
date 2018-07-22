#pragma once

#include "Material.hpp"
#include "PhongMaterial.hpp"

#include <glm/glm.hpp>

#include <list>
#include <string>
#include <iostream>
#include "Ray.hpp"
#include "ColInfo.hpp"


enum class NodeType {
	SceneNode,
	GeometryNode,
	JointNode
};

class SceneNode {
public:
    SceneNode(const std::string & name);

	SceneNode(const SceneNode & other);

    virtual ~SceneNode();
    
	int totalSceneNodes() const;
    
    const glm::mat4 get_timeTransform(float timeStep) const;
    const glm::mat4 get_transform(float timeStep) const;
    const glm::mat4 get_inverse(float timeStep) const;
    
    void set_transform(const glm::mat4& m);
    
    void add_child(SceneNode* child);
    
    void remove_child(SceneNode* child);

	//-- Transformations:
    void rotate(char axis, float angle);
    void scale(const glm::vec3& amount);
    void translate(const glm::vec3& amount);
    void setVelocity(const glm::vec3& amount);
    void setAcceleration(const glm::vec3& amount);
    void setRotationalVelocity(const glm::vec3& amount);
    virtual double getMaxRadius(float t);

    virtual bool hit(Ray &r, ColInfo &info, bool shortcut, float t);
    dvec4 backTraceNormal(dvec4 N,float t);

	friend std::ostream & operator << (std::ostream & os, const SceneNode & node);

    // Transformations
    glm::mat4 trans;
    glm::mat4 invtrans;
    
    std::list<SceneNode*> children;
    glm::vec3 m_velocity;
    glm::vec3 m_rotational_velocity;
    glm::vec3 m_acceleration;


	NodeType m_nodeType;
	std::string m_name;
	unsigned int m_nodeId;

    double cachedMaxR = -1.0;

private:
	// The number of SceneNode instances.
	static unsigned int nodeInstanceCount;
};
