#pragma once

#include <glm/glm.hpp>

#include "Material.hpp"

class PhongMaterial : public Material {
public:
  PhongMaterial(const glm::vec3& kd, const glm::vec3& ks, double shininess);
  virtual ~PhongMaterial();

  glm::vec3 m_kd;
  glm::vec3 m_ks;

  double m_shininess;
  double m_refractiveIndex = 1.0;
  double m_kr = 1.0f;
  double m_kt = 0.0f;

private:
  
};
