#include "renderer.h"
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
Renderer::Renderer(TGAImage &image, RenderOptions &options,
                   Model *model) noexcept
    : image_(image), options_(options),
      zbuffer_(std::make_unique<float[]>(options.width * options.height)),
      model_(model) {
  std::fill_n(zbuffer_.get(), options.width * options.height,
              -std::numeric_limits<float>::max());
}

Renderer::~Renderer() noexcept {
  // clear rendered image
  image_.clear();

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
 * @brief set image ref
 *
 * @param image new image to be set
 */
void Renderer::set_image(TGAImage &image) noexcept {
  // here clear image_ ref
  image_.clear();
  image_ = image;
}
/**
 * @brief set model ptr,this will only release the model ptr if pass nullptr
 *
 * @param model new model to be set
 */
void Renderer::set_model(Model *model) noexcept {
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
void Renderer::set_texture(TGAImage &texture, ShadingType type) noexcept {
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
void Renderer::set_options(RenderOptions &options) noexcept {
  options_ = options;
}

/**
 * @brief Render in wireframe mode with cached line
 *
 */
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

/**
 * @brief Render in gray image zbuffer mode
 *
 */
void Renderer::render_zbufgray() noexcept {
  Vec3f camera(1, 1, 3);     // define camera position;
  Vec3f obj_center(0, 0, 0); // define object center position
  Triangle cached_triangle(options_.shadingmode);

  TGAImage zbufimage(options_.width, options_.height, TGAImage::GRAYSCALE);

  Mat4f m_trans = model_trans();
  Mat4f v_trans = view_trans(camera, obj_center - camera, Vec3f(0, 1, 0));
  Mat4f p_trans = Mat4f::identity();
  p_trans[3][2] = -1.0f / camera.z;
  Mat4f viewport = viewport_trans(
      options_.width * 1 / 8, options_.height * 1 / 8, options_.width * 3 / 4,
      options_.height * 3 / 4, options_.depth);

  // render each piece/triangles
  for (int i = 0; i < model_->v_ind_num(); i++) {
    // load data from model_
    std::vector<int> face = model_->getv_ind(i);
    Vec3f screen_coords[3]; // coord of 3 verts trace on screen plate
    for (int j = 0; j < 3; j++) {
      Vec3f v = model_->getv(face[j]);
      screen_coords[j] = m2v(viewport * p_trans * v_trans * m_trans * v2m(v));
    }
    cached_triangle.set_rverts(screen_coords);

    // render on image, triangle as piece
    cached_triangle.draw(image_, zbuffer_.get());
  }

  // render finally z buffer preview image
  for (int i = 0; i < options_.width; i++) {
    for (int j = 0; j < options_.height; j++) {
      zbufimage.set(i, j, TGAColor(zbuffer_.get()[i + j * options_.width], 1));
    }
  }

  // we don't need the triangle image, so let's replace it.
  image_.clear();
  image_ = zbufimage;
}

/**
 * @brief Render in triangle piece mode with cached triangle
 *
 */
void Renderer::render_triangle() noexcept {
  Vec3f light_dir(0, 0, -1); // define light_dir
  Vec3f camera(1, 0, 3);     // define camera position
  Vec3f obj_center(0, 0, 0); // define object center position
  Triangle cached_triangle(options_.shadingmode);

  Mat4f m_trans = model_trans();
  Mat4f v_trans = view_trans(camera, obj_center - camera, Vec3f(0, 1, 0));
  // Mat4f p_trans = projection_trans(60, 16.0f / 9.0f, 1, 255);
  // Below is the simple projection matrix used before...
  Mat4f p_trans = Mat4f::identity();
  p_trans[3][2] = -1.0f / (camera - obj_center).norm();
  Mat4f viewport = viewport_trans(
      options_.width * 1 / 8, options_.height * 1 / 8, options_.width * 3 / 4,
      options_.height * 3 / 4, options_.depth);

  // render each face/piece
  for (int i = 0; i < model_->face_num(); i++) {
    // load data from model_
    std::vector<int> v_ind_tuple = model_->getv_ind(i);
    std::vector<int> vt_ind_tuple = model_->getvt_ind(i);
    std::vector<int> vn_ind_tuple = model_->getvn_ind(i);

    Vec3f screen_coords[3]; // coord of 3 verts trace on viewport plateform
    Vec3f world_coords[3];  // coord of 3 verts without any transform
    Vec2f tex_coords[3];    // coord of 3 verts for texturing
    Vec3f norm_coords[3];   // coord of 3 vertex for lighting

    for (int j = 0; j < 3; j++) {
      Vec3f v = model_->getv(v_ind_tuple[j]);
      Vec2f vt = model_->getvt(vt_ind_tuple[j]);
      Vec3f vn = model_->getvn(vn_ind_tuple[j]);

      world_coords[j] = v;
      screen_coords[j] = m2v(viewport * p_trans * v_trans * m_trans * v2m(v));
      tex_coords[j] = vt;
      norm_coords[j] = vn;
    }

    cached_triangle.set_verts(world_coords);
    cached_triangle.set_rverts(screen_coords);
    cached_triangle.set_uvs(tex_coords);
    cached_triangle.set_normals(norm_coords);

    // calculate light intensity...
    Vec3f n = (world_coords[2] - world_coords[0]) ^
              (world_coords[1] - world_coords[0]);
    n.normalize();
    float intensity = n * light_dir;

    // render on image, texturing will be done in draw_triangle()
    if (intensity > 0) {
      cached_triangle.draw(image_, zbuffer_.get(), diffusemap_, intensity);
    }
  }
}

/**
 * @brief Render regarding rendering mode(shading mode will be used in triangle
 * render)
 *
 */
void Renderer::render() noexcept {
  image_.clear();

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
  image_.flip_vertically();
}

/**
 * @brief Save image into the provided path in format of .tga image
 *
 * @param filename image to store the output, suffix should be .tga
 */
void Renderer::save_image(std::string filename) noexcept {
  image_.write_tga_file(filename.c_str());
}