#include "model.h"
#include "gmath.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

Model::Model(std::string filename)
    : v_(), v_tex_(), v_norm_(), v_indices_(), vt_indices_(), vn_indices_() {
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
      v_tex_.push_back(vt.toVec2());
    } else if (!line.compare(0, 4, "vn  ")) {
      Vec3f vn;
      iss >> trash >> trash; // skip "vn  "
      for (int i = 0; i < 3; i++)
        iss >> vn.raw[i];
      v_norm_.push_back(vn);
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

      v_indices_.push_back(vert_indices);
      vt_indices_.push_back(tex_indices);
      vn_indices_.push_back(norm_indices);
    }
  }
  std::cerr << "# verts sum as: " << v_.size() << "\n"
            << "# texture verts sum as: " << v_tex_.size() << "\n"
            << "# normal verts sum as: " << v_norm_.size() << "\n"
            << "# verts indices (faces) sum as: " << v_indices_.size() << "\n"
            << "# texture verts indices sum as: " << vt_indices_.size() << "\n"
            << "# normal verts indices sum as: " << vn_indices_.size() << "\n";
}
Model::~Model() {
  v_.clear();
  v_tex_.clear();
  v_norm_.clear();

  v_indices_.clear();
  vt_indices_.clear();
  vn_indices_.clear();
#ifdef DEBUG
  std::cerr << "Model destroyed" << std::endl;
#endif
}

int Model::v_num() const { return (int)v_.size(); }
int Model::vt_num() const { return (int)v_tex_.size(); }
int Model::vn_num() const { return (int)v_norm_.size(); }

int Model::face_num() const { return (int)v_indices_.size(); }
int Model::v_ind_num() const { return (int)v_indices_.size(); }
int Model::vt_ind_num() const { return (int)vt_indices_.size(); }
int Model::vn_ind_num() const { return (int)vn_indices_.size(); }

Vec3f Model::getv(int ind) const { return v_[ind]; }
Vec2f Model::getvt(int ind) const { return v_tex_[ind]; }
Vec3f Model::getvn(int ind) const { return v_norm_[ind]; }

std::vector<std::vector<int>> Model::getface(int ind) const {
  std::vector<std::vector<int>> f;
  std::vector<int> v_indices = getv_ind(ind);
  std::vector<int> vt_indices = getvt_ind(ind);
  std::vector<int> vn_indices = getvn_ind(ind);
  for (int i = 0; i < 3; i++)
    f.push_back(std::vector<int>{v_indices[i], vt_indices[i], vn_indices[i]});
  return f;
}
std::vector<int> Model::getv_ind(int ind) const { return v_indices_[ind]; }
std::vector<int> Model::getvt_ind(int ind) const { return vt_indices_[ind]; }
std::vector<int> Model::getvn_ind(int ind) const { return vn_indices_[ind]; }