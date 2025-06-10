#include "gmath.hpp"
#include "model.h"
#include "tgaimage.h"
#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>

void save_image(TGAImage &image);

Model *model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;

void line(Vec3i p0, Vec3i p1, TGAImage &image, TGAColor color) {
  // z for depth , but we currently igonre it in our line drawing process.
  bool steep = false;
  if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
    std::swap(p0.x, p0.y);
    std::swap(p1.x, p1.y);
    steep = true;
  }
  if (p0.x > p1.x) {
    std::swap(p0, p1);
  }

  for (int x = p0.x; x <= p1.x; x++) {
    float t = (x - p0.x) / (float)(p1.x - p0.x);
    int y = p0.y * (1. - t) + p1.y * t + .5;
    if (steep) {
      image.set_pixel(y, x, color);
    } else {
      image.set_pixel(x, y, color);
    }
  }
}

template <typename T>
constexpr Vec3<T> m2v(const Matrix<T, 4, 1> mat) noexcept {
  return Vec3<T>(mat[0][0] / mat[3][0], mat[1][0] / mat[3][0],
                 mat[2][0] / mat[3][0]);
}

template <typename T>
constexpr Matrix<T, 4, 1> v2m(const Vec3<T> vec) noexcept {
  Matrix<T, 4, 1> mat;
  mat[0][0] = vec.x;
  mat[1][0] = vec.y;
  mat[2][0] = vec.z;
  mat[3][0] = T(1.0f);
  return mat;
}

template <typename T>
constexpr Matrix<T, 4, 4> viewport_trans(int x, int y, int w, int h) noexcept {
  Matrix<T, 4, 4> m = Mat4<T>::identity();
  m[0][3] = x + w / 2.0f;
  m[1][3] = y + h / 2.0f;
  m[2][3] = depth / 2.0f;

  m[0][0] = w / 2.0f;
  m[1][1] = h / 2.0f;
  m[2][2] = depth / 2.0f;
  return m;
}

template <typename T>
constexpr Matrix<T, 4, 4> scale_trans(float value) noexcept {
  Matrix<T, 4, 4> m = Mat4<T>::identity();
  m[0][0] = m[1][1] = m[2][2] = value;
  return m;
}

template <typename T>
constexpr Matrix<T, 4, 4> translation_trans(Vec3f v) noexcept {
  // maybe also called affine transformation?
  Matrix<T, 4, 4> m = Mat4<T>::identity();
  m[0][3] = v.x;
  m[1][3] = v.y;
  m[2][3] = v.z;
}

template <typename T>
constexpr Matrix<T, 4, 4> x_rotate_trans(float angle_in_rad) noexcept {
  Mat4<T> m = Mat4<T>::identity();
  float cosangle = cos(angle_in_rad);
  float sinangle = sin(angle_in_rad);
  m[1][1] = m[2][2] = cosangle;
  m[1][2] = -sinangle;
  m[2][1] = sinangle;
  return m;
}

template <typename T>
constexpr Matrix<T, 4, 4> y_rotate_trans(float angle_in_rad) noexcept {
  Mat4<T> m = Mat4<T>::identity();
  float cosangle = cos(angle_in_rad);
  float sinangle = sin(angle_in_rad);
  m[0][0] = m[2][2] = cosangle;
  m[0][2] = -sinangle;
  m[2][0] = sinangle;
  return m;
}

template <typename T>
constexpr Matrix<T, 4, 4> z_rotate_trans(float angle_in_rad) noexcept {
  Mat4<T> m = Mat4<T>::identity();
  float cosangle = cos(angle_in_rad);
  float sinangle = sin(angle_in_rad);
  m[0][0] = m[1][1] = cosangle;
  m[0][1] = -sinangle;
  m[1][0] = sinangle;
  return m;
}

int main(int argc, char **argv) {
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/cube.obj");
  }

  TGAImage image(width, height, TGAImage::RGB);
  Matrix<float, 4, 4> VP =
      viewport_trans<float>(width / 4, width / 4, width / 2, height / 2);

  // draw the axes
  Vec3f x(1.f, 0.f, 0.f), y(0.f, 1.f, 0.f), o(0.f, 0.f, 0.f);
  o = m2v(VP * v2m(o));
  x = m2v(VP * v2m(x));
  y = m2v(VP * v2m(y));
  line(Vec3i(o), Vec3i(x), image, red);
  line(Vec3i(o), Vec3i(y), image, green);

  for (int i = 0; i < model->face_num(); i++) {
    std::vector<int> face = model->getv_ind(i);
    for (int j = 0; j < (int)face.size(); j++) {
      Vec3f wp0 = model->getv(face[j]);
      Vec3f wp1 = model->getv(face[(j + 1) % face.size()]);

      // draw the original model
      Vec3f sp0 = m2v(VP * v2m(wp0));
      Vec3f sp1 = m2v(VP * v2m(wp1));
      line(Vec3i(sp0), Vec3i(sp1), image, white);

      // draw the deformed model
      Matrix<float, 4, 4> transform = scale_trans<float>(1.5f);
      Vec3f dsp0 = m2v(VP * transform * v2m(wp0));
      Vec3f dsp1 = m2v(VP * transform * v2m(wp1));
      line(Vec3i(dsp0), Vec3i(dsp1), image, yellow);
    }
  }

  image.flip_vertically();
  save_image(image);
  delete model;
  return 0;
}

void save_image(TGAImage &image) {
  std::string filename;
  std::cout << "Picture rendered succeessfully!\nWhere to save the result:\n";
  std::cin >> filename;
  if (filename.empty() || filename.length() <= 4 ||
      filename.substr(filename.length() - 4, 4) != ".tga") {
    filename.append(".tga");
  }
  image.write_tga_file(filename.c_str());
}