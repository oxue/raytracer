#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include "Texture.hpp"
#include "ColInfo.hpp"

using namespace std;
using namespace glm;

int earlyReturn = 0;

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
	const std::string & name, Primitive *prim, Material *mat )
	: SceneNode( name )
	, m_material( mat )
	, m_primitive( prim )
{
	m_nodeType = NodeType::GeometryNode;
}

double GeometryNode::getMaxRadius(){
	return length(get_transform()*m_primitive->getPos()) + length(get_transform() * dvec4(m_primitive->maxRadius,0,0,1));
}

void GeometryNode::setMaterial( Material *mat )
{
	// Obviously, there's a potential memory leak here.  A good solution
	// would be to use some kind of reference counting, as in the 
	// C++ shared_ptr.  But I'm going to punt on that problem here.
	// Why?  Two reasons:
	// (a) In practice we expect the scene to be constructed exactly
	//     once.  There's no reason to believe that materials will be
	//     repeatedly overwritten in a GeometryNode.
	// (b) A ray tracer is a program in which you compute once, and 
	//     throw away all your data.  A memory leak won't build up and
	//     crash the program.

	m_material = mat;
}

void GeometryNode::setTexture( Texture * texture )
{
	m_material->texture = texture;
}

bool GeometryNode::hit(Ray &r, ColInfo &info, bool shortcut){
	if(shortcut && m_primitive->maxRadius > 0 && !m_primitive->skipCheck){
		Ray deepRay = {
			get_inverse() * r.origin,
			get_inverse() * r.head
		};
		NonhierSphere s = NonhierSphere({0, 0, 0}, m_primitive->maxRadius);
		ColInfo cf;
		if(!s.hit(deepRay, info)) {
			earlyReturn++;
			return false;
		}
	}

	Ray deepRay = {
		get_inverse() * r.origin,
		get_inverse() * r.head
	};
	if(m_primitive->hit(deepRay, info))
	{
		PhongMaterial* pMat = (PhongMaterial*)m_material;
		info.material = *pMat;
		if(pMat->texture != nullptr){
			info.material.m_kd = pMat->texture->getColor(info.uv.x, info.uv.y) * pMat->m_kd;//N.x/2.0 + 0.5,N.y/2.0 + 0.5);	
		}else
		{
			info.material.m_kd = pMat->m_kd;
		}
		info.N = backTraceNormal(info.N);

		return true;
	}
	return false;
}