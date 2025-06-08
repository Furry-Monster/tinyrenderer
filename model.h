#ifndef __MODEL_H__
#define __MODEL_H__

#include "gmath.h"
#include <vector>

// Only support .obj format Models
class Model {
private:
  // vertex properties
  std::vector<Vec3f> v_;      // vertex
  std::vector<Vec2f> v_tex_;  // texture vertex
  std::vector<Vec3f> v_norm_; // normal vertex

  // face properties
  std::vector<std::vector<int>> v_indices_;  // vertex indices
  std::vector<std::vector<int>> vt_indices_; // texture vertex indices
  std::vector<std::vector<int>> vn_indices_; // normal vertex indices

public:
  // constructors
  Model(const char *filename);
  ~Model();

  // get sizes
  int v_num() const;
  int vt_num() const;
  int vn_num() const;

  int face_num() const;
  int v_ind_num() const;
  int vt_ind_num() const;
  int vn_ind_num() const;

  // index find
  Vec3f getv(int ind) const;
  Vec2f getvt(int ind) const;
  Vec3f getvn(int ind) const;

  std::vector<std::vector<int>> getface(int ind) const;
  std::vector<int> getv_ind(int ind) const;
  std::vector<int> getvt_ind(int ind) const;
  std::vector<int> getvn_ind(int ind) const;
};

#endif