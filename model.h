#ifndef __MODEL_H__
#define __MODEL_H__

#include "gmath.hpp"
#include "tgaimage.h"
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// Only support .obj format Models
class Model {
private:
  // vertex properties
  std::vector<Vec3f> v_;  // vertex
  std::vector<Vec2f> vt_; // texture vertex
  std::vector<Vec3f> vn_; // normal vertex

  // face properties
  std::vector<std::vector<int>> f_vi_; // vertex indices
  std::vector<std::vector<int>> f_vti; // texture vertex indices
  std::vector<std::vector<int>> f_vni; // normal vertex indices

public:
  // constructors
  explicit Model(std::string filename);
  ~Model();

  // get sizes
  int v_num() const;
  int vt_num() const;
  int vn_num() const;

  int f_num() const;
  int f_vi_num() const;
  int f_vti_num() const;
  int f_vni_num() const;

  // getters
  Vec3f getv(int ind) const;
  Vec3f getv(int iface, int nth_vert) const;
  Vec2f getvt(int ind) const;
  Vec2f getvt(int iface, int nth_vert) const;
  Vec3f getvn(int ind) const;
  Vec3f getvn(int iface, int nth_vert) const;

  std::vector<std::vector<int>> getf(int ind) const;
  std::vector<int> getf_vi(int ind) const;
  std::vector<int> getf_vti(int ind) const;
  std::vector<int> getf_vni(int ind) const;
};

#endif