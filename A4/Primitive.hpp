#pragma once

#include <glm/glm.hpp>
#include "A4.hpp"
#include "ColInfo.hpp"

extern double EPSILON;

class Primitive {
public:
  int type;

  virtual ~Primitive();
  virtual bool hit(Ray &r, ColInfo& info);
  virtual glm::dvec4 getPos() {
    return glm::vec4(0,0,0,1.0);
  }

  Primitive *m_primitive;
  double maxRadius = -1.0;
  double skipCheck = true;
};

class Sphere : public Primitive {
public:
  Sphere();
  virtual ~Sphere();
  bool hit(Ray &r, ColInfo& info);
};

class Cube : public Primitive {
public:
  Cube();
  virtual ~Cube();
  bool hit(Ray &r, ColInfo& info);
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
    skipCheck = true;
    maxRadius = radius + 0.5;
  }
  glm::dvec4 getPos() {
    return glm::dvec4(m_pos, 1.0);
  }

  bool hit(Ray &r, ColInfo& info);
  virtual ~NonhierSphere();

private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
    skipCheck = true;
    maxRadius = glm::sqrt(size*size*3)*4;
  }
  
  glm::dvec4 getPos() {
    return glm::dvec4(m_pos, 1.0);
  }
  bool hit(Ray &r, ColInfo& info);
  virtual ~NonhierBox();

private:
  glm::vec3 m_pos;
  double m_size;
};
