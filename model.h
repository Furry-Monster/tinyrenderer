#ifndef __MODEL_H__
#define __MODEL_H__

#include "gmath.h"
#include <vector>

// Only support .obj format Models
class Model {
private:
  std::vector<Vec3f> verts_;
  std::vector<std::vector<int>> faces_;

public:
  Model(const char *filename);
  ~Model();
  int nverts();
  int nfaces();
  Vec3f vert(int ind);
  std::vector<int> face(int ind);
};

#endif