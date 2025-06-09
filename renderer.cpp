#include "renderer.h"
#include "gutils.h"
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

void Renderer::set_image(TGAImage &image) noexcept {
  // here clear image_ ref
  image_.clear();
  image_ = image;
}
void Renderer::set_model(Model *model) noexcept {
  // here clear model_ ptr
  delete model_;
  model_ = model;
}
void Renderer::set_options(RenderOptions &options) noexcept {
  options_ = options;
}

/**
 * @brief Get a matrix for transformation from NDC to Screen/Viewport
 *
 * @param x Viewport origin x value
 * @param y Viewport origin y value
 * @param w Viewport width
 * @param h Viewport height
 * @param depth Depth for Culling and Depth-Testing
 * @return Mat4f Viewport Transformation Matrix
 */
Mat4f viewport_trans(int x, int y, int w, int h, int depth) noexcept {
  Mat4f m = {
      {w / 2.0f, 0, 0, x + w / 2.0f},
      {0, h / 2.0f, 0, y + h / 2.0f},
      {0, 0, depth / 2.0f, depth / 2.0f},
      {0, 0, 0, 1},
  };
  return m;
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
void Renderer::render_triangle() noexcept {
  Vec3f light_dir(0, 0, -1); // define light_dir
  Vec3f camera(0, 0, 3);     // define camera position;

  // this is too simple, using MVP later...
  Mat4f projection = Mat4f::identity();
  projection[3][2] = -1.0f / camera.z;
  Mat4f viewport = viewport_trans(
      options_.width * 1 / 8, options_.height * 1 / 8, options_.width * 3 / 4,
      options_.height * 3 / 4, options_.depth);

  // render each piece/triangles
  for (int i = 0; i < model_->v_ind_num(); i++) {
    // load data from model
    std::vector<int> face = model_->getv_ind(i);
    Vec3f screen_coords[3]; // coord of 3 verts trace on screen plate
    Vec3f world_coords[3];  // coord of 3 verts without any transform
    for (int j = 0; j < 3; j++) {
      Vec3f v = model_->getv(face[j]);
      world_coords[j] = v;
      screen_coords[j] = m2v(viewport * projection * v2m(v));
    }

    // calculate light intensity
    Vec3f n = (world_coords[2] - world_coords[0]) ^
              (world_coords[1] - world_coords[0]);
    n.normalize();
    float intensity = n * light_dir;

    // render on image, triangle as piece
    if (intensity > 0) {
      draw_triangle(
          screen_coords, zbuffer_.get(), image_,
          TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
    }
  }
}
void Renderer::render_zbufgray() noexcept {
  Vec3f camera(0, 0, 3); // define camera position;
  TGAImage zbufimage(options_.width, options_.height, TGAImage::GRAYSCALE);

  Mat4f projection = Mat4f::identity();
  projection[3][2] = -1.0f / camera.z;
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
      screen_coords[j] = m2v(viewport * projection * v2m(v));
    }

    // render on image, triangle as piece
    draw_triangle(screen_coords, zbuffer_.get(), image_, white);
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
void Renderer::render_textured() noexcept {
  Vec3f light_dir(0, 0, -1); // define light_dir
  Vec3f camera(0, 0, 3);     // define camera position;

  Mat4f projection = Mat4f::identity();
  projection[3][2] = -1.0f / camera.z;
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
      screen_coords[j] = m2v(viewport * projection * v2m(v));
      tex_coords[j] = vt;
      norm_coords[j] = vn;
    }

    // calculate light intensity
    Vec3f n = (world_coords[2] - world_coords[0]) ^
              (world_coords[1] - world_coords[0]);
    n.normalize();
    float intensity = n * light_dir;

    // render on image, texturing will be done in draw_triangle()
    if (intensity > 0) {
      draw_triangle(screen_coords, tex_coords, zbuffer_, image_, *model_,
                    intensity);
    }
  }
}

void Renderer::render() noexcept {
  image_.clear();
  switch (options_.mode) {
  case LINE:
    render_wireframe();
    break;
  case TRIANGLE:
    render_triangle();
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