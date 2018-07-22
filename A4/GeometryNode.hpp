#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "Material.hpp"
#include "PhongMaterial.hpp"
#include "ColInfo.hpp"
#include "Texture.hpp"

class GeometryNode : public SceneNode {
public:
	GeometryNode( const std::string & name, Primitive *prim, 
		Material *mat = nullptr );

	void setMaterial( Material *material );
	void setTexture( Texture *texture);
	void setNormalMap( Texture *texture);

    bool hit(Ray &r, ColInfo &info, bool shortcut, float t);
	double getMaxRadius(float t);

	Material *m_material;
	Primitive *m_primitive;
};
