#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "model.h"
#include "tgaimage.h"
#include <memory>

enum ShadingType {
  DIFFUSE = 0x1,
  NORMAL = 0x10,
  SPECULAR = 0x100,
};

enum RenderingMode {
  LINE,
  TRIANGLE,
  ZBUF,
};

struct RenderOptions {
  RenderingMode mode = RenderingMode::TRIANGLE;
  unsigned int shadingmode = 0;

  std::string objpath = "obj/african_head.obj";
  std::string diffusepath = "texture/african_head_diffuse.tga";
  std::string normalpath = "texture/african_head_nm.tga";
  std::string specularpath = "texture/african_head_spec.tga";
  std::string outputpath = "output.tga";

  int width = 800;
  int height = 800;
  int depth = 255;
};

const TGAColor white(255, 255, 255, 255);
const TGAColor red(255, 0, 0, 255);

class Renderer {
private:
  TGAImage &image_;
  Model *model_;
  TGAImage diffusemap_;
  TGAImage normalmap_;
  TGAImage specularmap_;
  RenderOptions &options_;

  std::unique_ptr<float[]> zbuffer_;

public:
  // constructors
  explicit Renderer(TGAImage &image, RenderOptions &options,
                    Model *model = nullptr) noexcept;

  ~Renderer() noexcept;

  // getter/setter
  void set_image(TGAImage &image) noexcept;
  void set_model(Model *model) noexcept;
  void set_texture(TGAImage &texture, ShadingType type) noexcept;
  void set_options(RenderOptions &options) noexcept;

  // functions
  void render() noexcept;
  void save_output() noexcept;

private:
  void render_wireframe() noexcept;
  void render_triangle() noexcept;
  void render_zbufgray() noexcept;
};

#endif // __RENDERER_H__