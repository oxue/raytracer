#include <iostream>
#include <fstream>

#include <glm/ext.hpp>
#include <math.h>

//#include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"
using namespace glm;

Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;
	cout<<fname<<endl;
	type = 1337;
	skipCheck = false;

	std::string realName = "Assets/" + fname;

	std::ifstream ifs( realName.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}
	cout<<m_faces.size()<<endl;
	
	for (vec3 v:m_vertices){
		double r2 = v.x * v.x + v.y * v.y + v.z* v.z;
		if(glm::sqrt(r2) > maxRadius) {
			maxRadius = glm::sqrt(r2);
		}
	}
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}

bool Mesh::intersetTriangle(Triangle tr, Ray r, double &t, dvec4 &N)
{
	dvec3 a = m_vertices[tr.v1];
	dvec3 b = m_vertices[tr.v2];
	dvec3 c = m_vertices[tr.v3];
	dvec3 p = dvec3(r.origin.x,r.origin.y, r.origin.z);
	dvec3 q = p+dvec3(r.head.x, r.head.y, r.head.z);
	
	dvec3 ab = b-a;
	dvec3 ac = c-a;
	dvec3 qp = p-q;

	dvec3 n = cross(ab,ac);
	N = dvec4(n, 0.0);
	double d = dot(qp, n);
	if(d<=0.0000) return false;

	// Compute intersection t value of pq with plane of triangle. A ray intersects iff 0 <= t
	dvec3 ap = p-a;
	t = dot(ap, n);
	if(t<0.0000) return false;

	dvec3 e = cross(qp, ap);
	double v =  dot(ac, e);
	if(v<0.0000||v>d) return false;
	double w = -dot(ab,e);
	if(w <0.0000 ||w+v>d) return false;
	double ood = 1.0/d;
	t *= ood;

	return true;
}

bool Mesh::hit(Ray &r, double &t, glm::dvec4 &N)
{
	bool hit = false;
	t = INFINITY;
	for(Triangle tr:m_faces){
		
		double mt=t;
		dvec4 mN;
		if(intersetTriangle(tr, r, mt, mN)){
			if(mt<t){
				t = mt;
				N = normalize(mN);
			}
			hit = true;
		}
	}
	return hit;
}

