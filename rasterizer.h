#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__

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
  RenderOptions &options_;
  std::unique_ptr<float[]> zbuffer_;
  std::unique_ptr<TGAImage> frame_;

  Model *model_;
  TGAImage diffusemap_;
  TGAImage normalmap_;
  TGAImage specularmap_;

public:
  // constructors
  explicit Rasterizer(RenderOptions &options, Model *model = nullptr) noexcept;

  ~Rasterizer() noexcept;

  // getter/setter
  void set_model(Model *model) noexcept;
  void set_texture(TGAImage &texture, ShadingType type) noexcept;
  void set_options(RenderOptions &options) noexcept;

  // functions
  void render() noexcept;
  void save_image(std::string filename) noexcept;

private:
  void render_wireframe() noexcept;
  void render_zbufgray() noexcept;
  void render_triangle() noexcept;
};

#endif // __RASTERIZER_H__