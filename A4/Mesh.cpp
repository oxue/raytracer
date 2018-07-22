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
	, m_uvs()
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
			if(fname == "goldfish.obj") {
				vx *= 20;
				vy *= 20;
				vz *= 20;
			}
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
			m_uvs.push_back(glm::vec2(0.0f));
		} else if(code == "q") {
			double vu, vv;
			ifs >> vx >> vy >> vz >> vu >> vv;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
			m_uvs.push_back(glm::vec2(vu, vv));
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}
	cout<<"faces "<<m_faces.size()<<endl;
	
	for (vec3 v:m_vertices){
		double r2 = v.x * v.x + v.y * v.y + v.z* v.z;
		if(glm::sqrt(r2) > maxRadius) {
			maxRadius = glm::sqrt(r2);
		}
	}

	cout<<"max radius " << maxRadius<<endl;

	computeTangents();
}

void Mesh::computeTangents() {
	for(Triangle &tr:m_faces)
	{
		vec3 v1 = m_vertices[tr.v1];
		vec3 v2 = m_vertices[tr.v2];
		vec3 v3 = m_vertices[tr.v3];

		vec2 w1 = m_uvs[tr.v1];
		vec2 w2 = m_uvs[tr.v2];
		vec2 w3 = m_uvs[tr.v3];

		vec3 p1 = v2 - v1;
		vec3 p2 = v3 - v1;

		vec2 q1 = w2 - w1;
		vec2 q2 = w3 - w1;

		float r = 1.0f/(q1.x * q2.y - q2.x * q1.y);
		vec3 sdir(
			(q2.y * p1.x - q1.y * p2.x) * r,
			(q2.y * p1.y - q1.y * p2.y) * r,
			(q2.y * p1.z - q1.y * p2.z) * r);

		tr.tangent = normalize(sdir);
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

dvec3 Mesh::Barycentric(dvec4 p, dvec3 a, dvec3 b, dvec3 c)
{
	dvec3 ret;
    dvec3 v0 = b - a;
    dvec3 v1 = c - a;
    dvec3 v2 = dvec3(p.x, p.y, p.z) - a;
    double d00 = dot(v0, v0);
    double d01 = dot(v0, v1);
    double d11 = dot(v1, v1);
    double d20 = dot(v2, v0);
    double d21 = dot(v2, v1);
    double denom = d00 * d11 - d01 * d01;
    ret.y = (d11 * d20 - d01 * d21) / denom;
    ret.z = (d00 * d21 - d01 * d20) / denom;
    ret.x = 1.0f - ret.y - ret.z;

    return ret;
}

bool Mesh::intersectTriangle(Triangle tr, Ray r, ColInfo& info)
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
	info.N = normalize(dvec4(n, 0.0));
	double d = dot(qp, n);
	if(d<=0.0000) return false;

	// Compute intersection t value of pq with plane of triangle. A ray intersects iff 0 <= t
	dvec3 ap = p-a;
	info.t = dot(ap, n);
	if(info.t<0.0000) return false;

	dvec3 e = cross(qp, ap);
	double v =  dot(ac, e);
	if(v<0.0000||v>d) return false;
	double w = -dot(ab,e);
	if(w <0.0000 ||w+v>d) return false;
	double ood = 1.0/d;
	info.t *= ood;

	dvec2 uvCoordsA = m_uvs[tr.v1];
	dvec2 uvCoordsB = m_uvs[tr.v2];
	dvec2 uvCoordsC = m_uvs[tr.v3];

	dvec3 baryCoords = Barycentric(r.origin + r.head * info.t, a, b, c);
	info.uv = baryCoords.x * uvCoordsA + 
		baryCoords.y * uvCoordsB +
		baryCoords.z * uvCoordsC;

	info.tangent = tr.tangent;
	info.bitangent = normalize(cross(dvec3(tr.tangent), n));

	return true;
}

bool Mesh::hit(Ray &r, ColInfo& info)
{
	bool hit = false;
	info.t = INFINITY;
	for(Triangle tr:m_faces){
		ColInfo mInfo;
		if(intersectTriangle(tr, r, mInfo)){
			if(mInfo.t<info.t){
				info = mInfo;
			}
			hit = true;
		}
	}
	return hit;
}

