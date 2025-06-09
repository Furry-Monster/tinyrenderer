#include "renderer.h"
#include "primitive.h"
#include "tgaimage.h"
#include <algorithm>
#include <limits>
#include <memory>

Renderer::Renderer(TGAImage &image, RenderOptions &options,
                   Model *model) noexcept
    : image_(image), options_(options), model_(model),
      zbuffer_(std::make_unique<float[]>(options.width * options.height)) {
  std::fill_n(zbuffer_.get(), options.width * options.height,
              -std::numeric_limits<float>::max());
}

Renderer::~Renderer() noexcept {
  image_.clear();
  zbuffer_.release();
  delete model_;
  model_ = nullptr;
}

void Renderer::set_image(TGAImage &image) noexcept { image_ = image; }
void Renderer::set_model(Model *model) noexcept {
  // here clear model_ ptr
  delete model_;
  model_ = model;
}
void Renderer::set_options(RenderOptions &options) noexcept {
  options_ = options;
}

void Renderer::render_wireframe() noexcept {
  Line cached_line(white);
  for (int i = 0; i < model_->v_ind_num(); i++) {
    std::vector<int> face = model_->getv_ind(i);
    for (int j = 0; j < 3; j++) {
      Vec3f v0 = model_->getv(face[j]);
      Vec3f v1 = model_->getv(face[(j + 1) % 3]);
      int x0 = (v0.x + 1.) * options_.width / 2.;
      int y0 = (v0.y + 1.) * options_.height / 2.;
      int x1 = (v1.x + 1.) * options_.width / 2.;
      int y1 = (v1.y + 1.) * options_.height / 2.;
      cached_line.set_point(Vec2i(x0, y0), Vec2i(x1, y1));
    }
    cached_line.draw(image_, zbuffer_.get());
  }
}
void Renderer::render_triangles() noexcept {}
void Renderer::render_zbufgray() noexcept {}
void Renderer::render_textured() noexcept {}

void Renderer::render() noexcept {
  image_.clear();
  switch (options_.mode) {
  case LINE:
    render_wireframe();
    break;
  case TRIANGLE:
    render_triangles();
    break;
  case ZBUF:
    render_zbufgray();
    break;
  case TEXTURED:
    render_textured();
    break;
  case SHADING:
    std::cout << "Not implemented yet" << std::endl;
    break;
  }

  // flip image vertically ,
  // cuz the drawing in TGAImage is upside down.
  image_.flip_vertically();
}

void Renderer::save_output() noexcept {
  image_.write_tga_file(options_.outputpath.c_str());
}