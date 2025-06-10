#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__

#include "gmath.hpp"
#include "model.h"
#include "tgaimage.h"
#include <memory>

enum ShadingType {
  DIFFUSE = 0x1,
  NORMAL = 0x10,
  SPECULAR = 0x100,
};

enum RenderingMode {
  WIREFRAME,
  TRIANGLE,
  ZBUFGRAY,
};

struct RenderOptions {
  RenderingMode mode = RenderingMode::TRIANGLE;
  unsigned int shadingmode = 0;

  int width = 1080;
  int height = 1080;
  int depth = 255;
};

class Rasterizer {
private:
  // below block are resource needing clean
  RenderOptions &options_;
  std::unique_ptr<float[]> zbuffer_;
  std::unique_ptr<TGAImage> frame_;
  Model *model_;

  TGAImage diffusemap_;
  TGAImage normalmap_;
  TGAImage specularmap_;

  Vec3f light_dir = Vec3f(0, 0, -1);
  Vec3f camera = Vec3f(1, 0, 3);
  Vec3f obj_center = Vec3f(0, 0, 0);

  Mat4f m_trans;
  Mat4f v_trans;
  Mat4f p_trans;
  Mat4f viewport;
  bool is_mvp_calc = false;

public:
  // constructors
  explicit Rasterizer(RenderOptions &options, Model *model = nullptr) noexcept;

  ~Rasterizer() noexcept;

  // getter/setter
  Mat4f get_mvp() const noexcept;
  void set_model(Model *model) noexcept;
  void set_texture(TGAImage &texture, ShadingType type) noexcept;
  void set_options(RenderOptions &options) noexcept;

  // functions
  void render() noexcept;
  void save_image(std::string filename) noexcept;

private:
  void calc_mvp() noexcept;

  void render_wireframe() noexcept;
  void render_zbufgray() noexcept;
  void render_triangle() noexcept;
};

#endif // __RASTERIZER_H__