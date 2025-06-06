#include "model.h"
#include "gmath.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

Model::Model(const char *filename) : verts_(), faces_() {
  std::ifstream in;
  in.open(filename, std::ifstream::in);
  if (in.fail())
    return;
  std::string line;
  while (!in.eof()) {
    std::getline(in, line);
    std::istringstream iss(line.c_str());
    char trash;
    if (!line.compare(0, 2, "v ")) {
      iss >> trash; // skip "v "
      Vec3f v;
      for (int i = 0; i < 3; i++)
        iss >> v.raw[i];
      verts_.push_back(v);
    } else if (!line.compare(0, 2, "f ")) {
      std::vector<int> f;
      int itrash, idx;
      iss >> trash; // skip "f "
      while (iss >> idx >> trash >> itrash >> trash >> itrash) {
        // read in format of "f xxx/xxx/xxx xxx/xxx/xxx xxx/xxx/xxx"
        idx--; // idx start from 1 , but c++ array start from 0
        f.push_back(idx);
      }
      faces_.push_back(f);
    } else if (!line.compare(0, 3, "vt ")) {
      iss >> trash; // skip "vt "
      Vec3f vt;
      for (int i = 0; i < 3; i++)
        iss >> vt.raw[i];
      tex_verts_.push_back(vt.toVec2());
    } else if (!line.compare(0, 3, "vn ")) {
      iss >> trash; // skip "vn "
      Vec3f vn;
      for (int i = 0; i < 3; i++)
        iss >> vn.raw[i];
      norm_verts_.push_back(vn);
    }
  }
  std::cerr << "# verts sum as: " << verts_.size() << "\n"
            << "# texture verts sum as: " << tex_verts_.size() << "\n"
            << "# normal verts sum as: " << norm_verts_.size() << "\n"
            << "# faces sum as: " << faces_.size() << std::endl;
}
Model::~Model() {
  verts_.clear();
  tex_verts_.clear();
  norm_verts_.clear();
  faces_.clear();
#ifdef DEBUG
  std::cerr << "Model destroyed" << std::endl;
#endif
}

int Model::nverts() { return (int)verts_.size(); }
int Model::ntexverts() { return (int)tex_verts_.size(); }
int Model::nnormverts() { return (int)norm_verts_.size(); }
int Model::nfaces() { return (int)faces_.size(); }

Vec3f Model::vert(int ind) { return verts_[ind]; }
Vec2f Model::texvert(int ind) { return tex_verts_[ind]; }
Vec3f Model::normvert(int ind) { return norm_verts_[ind]; }
std::vector<int> Model::face(int ind) { return faces_[ind]; }