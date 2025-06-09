#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "model.h"
#include "tgaimage.h"
#include <memory>

enum RenderingMode {
  LINE,
  TRIANGLE,
  ZBUF,
  TEXTURED,
  SHADING,
};

struct RenderOptions {
  RenderingMode mode = RenderingMode::TEXTURED;

  std::string objpath = "obj/african_head.obj";
  std::string diffusepath = "texture/african_head_diffuse.tga";
  std::string normalpath = "texture/african_head_nm.tga";
  std::string specularpath = "texture/african_head_spec.tga";
  std::string outputpath = "output.tga";

  int width = 800;
  int height = 800;
  int depth = 255;
};

class Renderer {
private:
  TGAImage &image_;
  Model *model_;
  RenderOptions &options_;

  std::unique_ptr<float[]> zbuffer_;

public:
  // constructors
  constexpr explicit Renderer(TGAImage &image, RenderOptions &options,
                              Model *model = nullptr) noexcept;

  ~Renderer() noexcept;

  // getter/setter
  constexpr void set_image(TGAImage &image) noexcept;
  constexpr void set_model(Model *model) noexcept;
  constexpr void set_options(RenderOptions &options) noexcept;

  // functions
  void render() noexcept;

  void render_wireframe() noexcept;
  void render_triangles() noexcept;
  void render_zbufgray() noexcept;
  void render_textured() noexcept;
};

#endif // __RENDERER_H__