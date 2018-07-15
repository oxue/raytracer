#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "PhongMaterial.hpp"
#include "Material.hpp"

class GeometryNode : public SceneNode {
public:
	GeometryNode( const std::string & name, Primitive *prim, 
		Material *mat = nullptr );

	void setMaterial( Material *material );

	bool hit(Ray &r, double &t, glm::dvec4 &N, PhongMaterial &material, bool shortcut = false);
	double getMaxRadius();

	Material *m_material;
	Primitive *m_primitive;
};
