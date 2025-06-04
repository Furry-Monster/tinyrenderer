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
      iss >> trash;
      while (iss >> idx >> trash >> itrash >> trash >> itrash) {
        idx--; // idx start from 1 , but c++ array start from 0
        f.push_back(idx);
      }
      faces_.push_back(f);
    }
  }
  std::cerr << "# verts sum as: " << verts_.size() << "\n"
            << "# faces sum as: " << faces_.size() << std::endl;
}
Model::~Model() {}

int Model::nverts() { return (int)verts_.size(); }
int Model::nfaces() { return (int)faces_.size(); }

Vec3f Model::vert(int ind) { return verts_[ind]; }
std::vector<int> Model::face(int ind) { return faces_[ind]; }