#pragma once

#include "Texture.hpp"

class Material {
public:
  virtual ~Material();
  Texture *texture = 0;
  Texture *normalMap = 0;

protected:
  Material();
};
