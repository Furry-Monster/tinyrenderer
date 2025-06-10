#include "model.h"
#include "gmath.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

Model::Model(std::string filename)
    : v_(), vt_(), vn_(), f_vi_(), f_vti_(), f_vni_() {
  std::ifstream in;
  in.open(filename, std::ifstream::in);
  if (in.fail())
    return;

  std::string line;
  while (!in.eof()) {
    // read line by line
    std::getline(in, line);
    std::istringstream iss(line.c_str());
    char trash;

    // parse this line with string stream
    if (!line.compare(0, 2, "v ")) {
      Vec3f v;
      iss >> trash; // skip "v "
      for (int i = 0; i < 3; i++)
        iss >> v.raw[i];
      v_.push_back(v);
    } else if (!line.compare(0, 4, "vt  ")) {
      Vec3f vt;
      iss >> trash >> trash; // skip "vt  "
      for (int i = 0; i < 3; i++)
        iss >> vt.raw[i];
      vt_.push_back(vt.toVec2());
    } else if (!line.compare(0, 4, "vn  ")) {
      Vec3f vn;
      iss >> trash >> trash; // skip "vn  "
      for (int i = 0; i < 3; i++)
        iss >> vn.raw[i];
      vn_.push_back(vn);
    } else if (!line.compare(0, 2, "f ")) {
      std::vector<int> vert_indices, tex_indices, norm_indices;
      int vert_idx, tex_idx, norm_idx;

      iss >> trash; // skip "f "
      while (iss >> vert_idx >> trash >> tex_idx >> trash >> norm_idx) {
        // read in format of "f xxx/xxx/xxx xxx/xxx/xxx xxx/xxx/xxx"
        // idx start from 1 , but c++ array start from 0
        vert_idx--;
        tex_idx--;
        norm_idx--;

        vert_indices.push_back(vert_idx);
        tex_indices.push_back(tex_idx);
        norm_indices.push_back(norm_idx);
      }

      f_vi_.push_back(vert_indices);
      f_vti_.push_back(tex_indices);
      f_vni_.push_back(norm_indices);
    }
  }
  std::cerr << "# verts sum as: " << v_.size() << "\n"
            << "# texture verts sum as: " << vt_.size() << "\n"
            << "# normal verts sum as: " << vn_.size() << "\n"
            << "# verts indices (faces) sum as: " << f_vi_.size() << "\n"
            << "# texture verts indices sum as: " << f_vti_.size() << "\n"
            << "# normal verts indices sum as: " << f_vni_.size() << "\n";
}
Model::~Model() {
  v_.clear();
  vt_.clear();
  vn_.clear();

  f_vi_.clear();
  f_vti_.clear();
  f_vni_.clear();
#ifdef DEBUG
  std::cerr << "Model destroyed" << std::endl;
#endif
}

int Model::v_num() const { return (int)v_.size(); }
int Model::vt_num() const { return (int)vt_.size(); }
int Model::vn_num() const { return (int)vn_.size(); }

int Model::f_num() const { return (int)f_vi_.size(); }
int Model::f_vi_num() const { return (int)f_vi_.size(); }
int Model::f_vti_num() const { return (int)f_vti_.size(); }
int Model::f_vni_num() const { return (int)f_vni_.size(); }

Vec3f Model::getv(int ind) const { return v_[ind]; }
Vec2f Model::getvt(int ind) const { return vt_[ind]; }
Vec3f Model::getvn(int ind) const { return vn_[ind]; }

std::vector<std::vector<int>> Model::getf(int ind) const {
  std::vector<std::vector<int>> f;
  std::vector<int> v_indices = f_vi_[ind];
  std::vector<int> vt_indices = f_vti_[ind];
  std::vector<int> vn_indices = f_vni_[ind];
  for (int i = 0; i < 3; i++)
    f.push_back(std::vector<int>{v_indices[i], vt_indices[i], vn_indices[i]});
  return f;
}

std::vector<int> Model::getf_vi(int ind) const { return f_vi_[ind]; }
std::vector<int> Model::getf_vti(int ind) const { return f_vti_[ind]; }
std::vector<int> Model::getf_vni(int ind) const { return f_vni_[ind]; }

Vec3f Model::getv(int iface, int nth_vert) const {
  return v_[f_vi_[iface][nth_vert]];
}
Vec2f Model::getvt(int iface, int nth_vert) const {
  return vt_[f_vti_[iface][nth_vert]];
}
Vec3f Model::getvn(int iface, int nth_vert) const {
  return vn_[f_vni_[iface][nth_vert]];
}
