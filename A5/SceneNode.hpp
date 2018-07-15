#pragma once

#include "Material.hpp"
#include "PhongMaterial.hpp"

#include <glm/glm.hpp>

#include <list>
#include <string>
#include <iostream>
#include "Ray.hpp"


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
    
    const glm::mat4& get_transform() const;
    const glm::mat4& get_inverse() const;
    
    void set_transform(const glm::mat4& m);
    
    void add_child(SceneNode* child);
    
    void remove_child(SceneNode* child);

	//-- Transformations:
    void rotate(char axis, float angle);
    void scale(const glm::vec3& amount);
    void translate(const glm::vec3& amount);
    virtual double getMaxRadius();

    virtual bool hit(Ray &r, double &t, glm::dvec4 &N, PhongMaterial &material, bool shortcut);

	friend std::ostream & operator << (std::ostream & os, const SceneNode & node);

    // Transformations
    glm::mat4 trans;
    glm::mat4 invtrans;
    
    std::list<SceneNode*> children;

	NodeType m_nodeType;
	std::string m_name;
	unsigned int m_nodeId;

    double cachedMaxR = -1.0;

private:
	// The number of SceneNode instances.
	static unsigned int nodeInstanceCount;
};