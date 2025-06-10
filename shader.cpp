#include "shader.h"

IShader::~IShader() {}

Vec3i GouraudShader::vertex(int iface, int nth_vert) {}

bool GouraudShader::fragment(Vec3f bar, TGAColor &color) {}