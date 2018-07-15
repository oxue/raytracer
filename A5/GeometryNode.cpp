#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"
#include <glm/glm.hpp>
#include <iostream>
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
	return length(get_transform() * m_primitive->getPos()) + length(get_transform() * dvec4(m_primitive->maxRadius,0,0,1));
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

bool GeometryNode::hit(Ray &r, double &t, dvec4 &N, PhongMaterial &material, bool shortcut){
	if(shortcut && m_primitive->maxRadius > 0 && !m_primitive->skipCheck){
		Ray deepRay = {
			get_inverse() * r.origin,
			get_inverse() * r.head
		};
		NonhierSphere s = NonhierSphere({0, 0, 0}, m_primitive->maxRadius);
		if(!s.hit(deepRay, t, N)) {
			earlyReturn++;
			return false;
		}
	}

	Ray deepRay = {
		get_inverse() * r.origin,
		get_inverse() * r.head
	};
	if(m_primitive->hit(deepRay, t, N))
	{
		PhongMaterial* pMat = (PhongMaterial*)m_material;
		material.m_ks = pMat->m_ks;
		material.m_kd = pMat->m_kd;
		material.m_shininess = pMat->m_shininess;
		N = vec4(transpose(dmat3(get_inverse())) * dvec3(N),0.0);
		return true;
	}
	return false;
}