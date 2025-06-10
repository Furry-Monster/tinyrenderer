#ifndef __SHADER_H__
#define __SHADER_H__

#include "gmath.hpp"
#include "tgaimage.h"
#include <algorithm>

struct IShader {
  virtual ~IShader();
  virtual Vec3i vertex(int iface, int nth_vert) = 0;
  virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

struct GouraudShader : public IShader {
  Vec3f varying_intensity; // passed from vertex shader to fragment shader

  virtual Vec3i vertex(int iface, int nth_vert);
  virtual bool fragment(Vec3f bar, TGAColor &color);
};

#endif // __SHADER_H__