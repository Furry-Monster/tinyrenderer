#ifndef __SHADER_H__
#define __SHADER_H__

#include "gmath.h"
#include "tgaimage.h"

struct IShader {
  virtual ~IShader();
  virtual Vec3i vertex(int iface, int nth_vert) = 0;
  virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

struct GouraudShader : public IShader {};

#endif // __SHADER_H__