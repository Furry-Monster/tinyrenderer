#include "rasterizer.h"
#include "gmath.hpp"
#include "gutils.hpp"
#include "primitive.hpp"
#include "tgaimage.h"
#include <algorithm>
#include <limits>
#include <memory>

/**
 * @brief Construct a new Renderer:: Renderer object,zbuffer will be
 * automatically initialized
 *
 * @param image image ref for renderer
 * @param options options ref for renderer
 * @param model model ptr for renderer,this will be completly managed by
 * renderer after passing
 */
Rasterizer::Rasterizer(RenderOptions &options, Model *model) noexcept
    : options_(options),
      zbuffer_(std::make_unique<float[]>(options.width * options.height)),
      frame_(std::make_unique<TGAImage>(options.width, options.height,
                                        TGAImage::RGB)),
      model_(model) {
  std::fill_n(zbuffer_.get(), options.width * options.height,
              -std::numeric_limits<float>::max());
  is_mvp_calc = false;
}

Rasterizer::~Rasterizer() noexcept {
  // clear rendered image
  frame_.get()->clear();
  frame_.reset();

  // clear textures
  diffusemap_.clear();
  normalmap_.clear();
  specularmap_.clear();

  // release buffer
  zbuffer_.reset();

  // clear model
  delete model_;
  model_ = nullptr;
}

/**
 * @brief Get the complete MVP matrix
 *
 * @return Mat4f
 */
Mat4f Rasterizer::get_mvp() const noexcept {
  return is_mvp_calc ? viewport * p_trans * v_trans * m_trans
                     : Mat4f::identity();
}

/**
 * @brief set model ptr,this will only release the model ptr if pass nullptr
 *
 * @param model new model to be set
 */
void Rasterizer::bind_model(Model *model) noexcept {
  // here clear model_ ptr
  delete model_;
  model_ = model;
}
/**
 * @brief set texture map for specific shading type
 *
 * @param texture TGAImage type image for texture
 * @param type shading type the texture will be used for
 */
void Rasterizer::bind_texture(TGAImage &texture, ShadingType type) noexcept {
  texture.flip_vertically();

  if (type == ShadingType::DIFFUSE) {
    diffusemap_.clear();
    diffusemap_ = texture;
  } else if (type == ShadingType::NORMAL) {
    normalmap_.clear();
    normalmap_ = texture;
  } else if (type == ShadingType::SPECULAR) {
    specularmap_.clear();
    specularmap_ = texture;
  }
}
void Rasterizer::bind_options(RenderOptions &options) noexcept {
  options_ = options;
}

void Rasterizer::calc_mvp() noexcept {
  m_trans = model_trans();
  v_trans = view_trans(camera, obj_center - camera, Vec3f(0, 1, 0));
  p_trans = Mat4f::identity();
  p_trans[3][2] = -1.0f / camera.z;
  viewport = viewport_trans(options_.width * 1 / 8, options_.height * 1 / 8,
                            options_.width * 3 / 4, options_.height * 3 / 4,
                            options_.depth);
  is_mvp_calc = true;
}

/**
 * @brief Render in wireframe mode with cached line
 *
 */
void Rasterizer::render_wireframe() noexcept {
  Line cached_line(white);
  for (int i = 0; i < model_->f_vi_num(); i++) {
    std::vector<int> face = model_->getf_vi(i);
    for (int j = 0; j < 3; j++) {
      Vec3f v0 = model_->getv(face[j]);
      Vec3f v1 = model_->getv(face[(j + 1) % 3]);
      int x0 = (v0.x + 1.) * options_.width / 2.;
      int y0 = (v0.y + 1.) * options_.height / 2.;
      int x1 = (v1.x + 1.) * options_.width / 2.;
      int y1 = (v1.y + 1.) * options_.height / 2.;
      cached_line.set_point(Vec2i(x0, y0), Vec2i(x1, y1));
    }
    cached_line.draw(*(frame_.get()), zbuffer_.get());
  }
}

/**
 * @brief Render in gray image zbuffer mode
 *
 */
void Rasterizer::render_zbufgray() noexcept {
  Triangle cached_triangle(options_.shadingmode);
  TGAImage zbufimage(options_.width, options_.height, TGAImage::GRAYSCALE);

  Vec3f screen_coords[3]; // coord of 3 verts trace on screen plate

  // render each piece/triangles
  for (int i = 0; i < model_->f_vi_num(); i++) {
    for (int j = 0; j < 3; j++) {
      screen_coords[j] = m2v3(get_mvp() * v2m(model_->getv(i, j)));
    }
    cached_triangle.set_rverts(screen_coords);

    // render on image, triangle as piece
    cached_triangle.draw(*(frame_.get()), zbuffer_.get());
  }

  // render finally z buffer preview image
  for (int i = 0; i < options_.width; i++) {
    for (int j = 0; j < options_.height; j++) {
      zbufimage.set_pixel(i, j,
                          TGAColor(zbuffer_.get()[i + j * options_.width], 1));
    }
  }

  // we don't need the triangle image, so let's replace it.
  frame_.get()->clear();
  frame_.reset();
  frame_ = std::make_unique<TGAImage>(std::move(zbufimage));
}

/**
 * @brief Render in triangle piece mode with cached triangle
 *
 */
void Rasterizer::render_triangle() noexcept {
  Triangle cached_triangle(options_.shadingmode);

  Vec3f screen_coords[3]; // coord of 3 verts trace on viewport plateform
  Vec3f world_coords[3];  // coord of 3 verts without any transform
  Vec2f tex_coords[3];    // coord of 3 verts for texturing
  Vec3f norm_coords[3];   // coord of 3 vertex for lighting

  // render each face/piece
  for (int i = 0; i < model_->f_num(); i++) {
    for (int j = 0; j < 3; j++) {
      world_coords[j] = model_->getv(i, j);
      screen_coords[j] = m2v3(get_mvp() * v2m(world_coords[j]));
      tex_coords[j] = model_->getvt(i, j);
      norm_coords[j] = model_->getvn(i, j);
    }

    cached_triangle.set_verts(world_coords);
    cached_triangle.set_rverts(screen_coords);
    cached_triangle.set_uvs(tex_coords);
    cached_triangle.set_normals(norm_coords);

    // render on image, texturing will be done in draw_triangle()
    cached_triangle.draw(*(frame_.get()), zbuffer_.get(), diffusemap_,
                         normalmap_, specularmap_);
  }
}

/**
 * @brief Render regarding rendering mode(shading mode will be used in triangle
 * render)
 *
 */
void Rasterizer::render() noexcept {
  frame_.get()->clear();
  if (!is_mvp_calc)
    calc_mvp();

  switch (options_.mode) {
  case WIREFRAME:
    render_wireframe();
    break;
  case ZBUFGRAY:
    render_zbufgray();
    break;
  case TRIANGLE:
    render_triangle();
    break;
  }

  // flip image vertically ,
  // cuz the drawing in TGAImage is upside down.
  frame_.get()->flip_vertically();
}

/**
 * @brief Save image into the provided path in format of .tga image
 *
 * @param filename image to store the output, suffix should be .tga
 */
void Rasterizer::save_frame(std::string filename) noexcept {
  frame_.get()->write_tga_file(filename.data());
}