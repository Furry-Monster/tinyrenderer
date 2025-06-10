#include "shader.h"
#include "gmath.hpp"
#include "gutils.hpp"
#include "tgaimage.h"
#include <algorithm>

IHardShader::~IHardShader() {}

GouraudShader::GouraudShader(Model &model, Vec3f &light_dir, Rasterizer &rst)
    : model(model), light_dir(light_dir), rst(rst) {}

Vec4f GouraudShader::vertex_exec(int iface, int nth_vert) {
  varying_intensity.raw[nth_vert] =
      std::max(0.0f, model.getvn(iface, nth_vert) * light_dir);
  Vec4f g_vertex = Vec4f(model.getv(iface, nth_vert));
  return m2v4(rst.get_mvp() * v2m(g_vertex));
}

bool GouraudShader::fragment_exec(Vec3f bc, TGAColor &color) {
  float intensity = varying_intensity * bc;
  color = TGAColor(255, 255, 255, 255) * intensity;
  return false;
}