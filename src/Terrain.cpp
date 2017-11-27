
#include "Terrain.h"
#include "GLSL.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "stb_image.h"

using namespace std;

void Terrain::init(float height)
{
	// Load texture
	//int index = 0;
	int w, h, ncomps;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(filename.c_str(), &w, &h, &ncomps, 0);
	if(! data)
	{
		cerr << filename << " not found" << endl;
	}
	width = w; // cols
	length = h; // rows

  heights.resize(length);
  for(int i = 0; i < length; i++)
  {
    heights[i].resize(width);
  }

  normals.resize(length);
  for(int i = 0; i < length; i++)
  {
    normals[i].resize(width);
  }

  for(int i = 0; i < length; i++)
  {
    for(int j = 0; j < width; j++)
    {
      int color = data[3 * (i * width + j)];
      float h = height * ((color / 255.0f) - 0.5f);
      heights[i][j] = h;
    }
  }

  // for(int i = 0; i < length; i++)
  // {
  //   for(int j = 0; j < width; j++)
  //   {
  //     printf("DEBUG %f, Index %d\n", heights[i][j], index++);
  //   }
  // }

  computedNorms = false;
  calcNormals();
  stbi_image_free(data);
}

void Terrain::calcNormals()
{
  if(computedNorms)
    return;

  for(int z = 0; z < length; z++)
  {
    for(int x= 0; x < width; x++)
    {
      vec3 sum = vec3(0.0f, 0.0f, 0.0f);

      vec3 out;
      if(z > 0)
        out = vec3(0.0f, heights[z - 1][x] - heights[z][x], -1.0f);

      vec3 in;
      if(z < length - 1)
        in = vec3(0.0f, heights[z + 1][x] - heights[z][x], 1.0f);

      vec3 left;
      if(x > 0)
        left = vec3(-1.0f, heights[z][x - 1] - heights[z][x], 0.0f);

      vec3 right;
      if(x < width - 1)
        right = vec3(1.0f, heights[z][x + 1] - heights[z][x], 0.0f);

      if(x > 0 && z > 0)
        sum += normalize(cross(out, left));

      if(x > 0 && z < length - 1)
        sum += normalize(cross(left, in));

      if(x < width - 1 && z < length - 1)
        sum += normalize(cross(in, right));

      if(x < width - 1 && z > 0)
        sum += normalize(cross(right, out));
      normals[z][x] = sum;
    }
  }
  computedNorms = true;
}

int Terrain::getWidth()
{
  return width;
}

int Terrain::getLength()
{
  return length;
}

void Terrain::setHeight(int x, int z, float y)
{
  heights[z][x] = y;
  computedNorms = false;
}

float Terrain::getHeight(int x, int z)
{
  return heights[z][x];
}

vec3 Terrain::getNormal(int x, int z)
{
  if(!computedNorms)
    calcNormals();
  return normals[z][x];
}
