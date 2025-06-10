#ifndef __SHADER_H__
#define __SHADER_H__

#include "gmath.hpp"
#include "tgaimage.h"
#include <algorithm>

//------------------------ Hard Shader Definitions ------------------------

struct IHardShader {
  // i'd like to call it hard shader actually,
  // cuz i think it's similar to the concept of hardcode values
  // :-)
  virtual ~IHardShader();
  virtual Vec4f vertex_exec(int iface, int nth_vert) = 0;
  virtual bool fragment_exec(Vec3f bar, TGAColor &color) = 0;
};

struct GouraudShader : public IHardShader {
  Vec3f varying_intensity; // passed from vertex shader to fragment shader

  virtual Vec4f vertex_exec(int iface, int nth_vert);
  virtual bool fragment_exec(Vec3f bar, TGAColor &color);
};

#endif // __SHADER_H__