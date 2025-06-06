#ifndef __MODEL_H__
#define __MODEL_H__

#include "gmath.h"
#include <vector>

// Only support .obj format Models
class Model {
private:
  std::vector<Vec3f> verts_;            // vertex
  std::vector<Vec2f> tex_verts_;        // texture vertex
  std::vector<Vec3f> norm_verts_;       // normal vertex
  std::vector<std::vector<int>> faces_; // faces

public:
  // constructors
  Model(const char *filename);
  ~Model();

  // get sizes
  int nverts();
  int ntexverts();
  int nnormverts();
  int nfaces();

  // index find
  Vec3f vert(int ind);
  Vec2f texvert(int ind);
  Vec3f normvert(int ind);
  std::vector<int> face(int ind);
};

#endif