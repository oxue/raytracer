#pragma once

#include <vector>
#include <iosfwd>
#include <string>

#include <glm/glm.hpp>

#include "Primitive.hpp"
#include "ColInfo.hpp"

struct Triangle
{
	size_t v1;
	size_t v2;
	size_t v3;

	Triangle( size_t pv1, size_t pv2, size_t pv3 )
		: v1( pv1 )
		, v2( pv2 )
		, v3( pv3 )
	{}
};

// A polygonal mesh.
class Mesh : public Primitive {
public:
  Mesh( const std::string& fname );

  bool hit(Ray &r, ColInfo& info);

private:
	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec2> m_uvs;
	std::vector<Triangle> m_faces;
	bool intersectTriangle(Triangle tr, Ray r, ColInfo& info);
	dvec3 Barycentric(dvec4 p, dvec3 a, dvec3 b, dvec3 c);

    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};
