#pragma once

#ifndef TERRAIN_H
#define TERRAIN_H

#include <iostream>
#include <glad/glad.h>
#include <vector>
#include "GLSL.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Terrain
{

public:

  void setFilename(const std::string &f) { filename = f; }
	void init(float height);
  int getWidth();
  int getLength();
  void setHeight(int x, int z, float y);
  float getHeight(int x, int z);
  void calcNormals();
  vec3 getNormal(int x, int z);


private:

  std::string filename;
  int width = 0;
  int length = 0;
  std::vector<std::vector<float>> heights;
  std::vector<std::vector<vec3>> normals;
  bool computedNorms;
};
#endif
