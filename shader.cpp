#include "shader.h"

IHardShader::~IHardShader() {}

Vec4f GouraudShader::vertex_exec(int iface, int nth_vert) {}

bool GouraudShader::fragment_exec(Vec3f bar, TGAColor &color) {}