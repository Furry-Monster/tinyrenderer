#include "renderer.h"
#include "tgaimage.h"
#include <algorithm>
#include <limits>
#include <memory>

constexpr Renderer::Renderer(TGAImage &image, RenderOptions &options,
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

constexpr void Renderer::set_image(TGAImage &image) noexcept { image_ = image; }
constexpr void Renderer::set_model(Model *model) noexcept { model_ = model; }
constexpr void Renderer::set_options(RenderOptions &options) noexcept {
  options_ = options;
}

void Renderer::render_wireframe() noexcept {}
void Renderer::render_triangles() noexcept {}
void Renderer::render_zbufgray() noexcept {}
void Renderer::render_textured() noexcept {}

void Renderer::render() noexcept {}