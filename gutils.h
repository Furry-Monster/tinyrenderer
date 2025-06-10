#ifndef __GUTILS_H__
#define __GUTILS_H__

#define MY_PI 3.14159f

#include "gmath.h"
#include <cmath>

/**
 * @brief translate 4*1 matrix to 1*3 vector
 *
 * @tparam T type
 * @param m matrix to translate
 * @return constexpr Vec3<T> translated vector
 */
template <typename T>
constexpr static Vec3<T> m2v(const Matrix<T, 4, 1> m) noexcept {
  // WARNING: if you pass a non-4*1 matrix, this still works well...
  return Vec3<T>(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

/**
 * @brief translate 1*3 vector to 4*1 matrix
 *
 * @tparam T type
 * @param v vector to translate
 * @return constexpr Mat4<T> translated matrix
 */
template <typename T>
constexpr static Matrix<T, 4, 1> v2m(const Vec3<T> v) noexcept {
  Matrix<T, 4, 1> m;
  m[0][0] = v.x;
  m[1][0] = v.y;
  m[2][0] = v.z;
  m[3][0] = T(1.0f);
  return m;
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
static Mat4f viewport_trans(int x, int y, int w, int h, int depth) noexcept {
  Mat4f m = {
      {w / 2.0f, 0, 0, x + w / 2.0f},
      {0, h / 2.0f, 0, y + h / 2.0f},
      {0, 0, depth / 2.0f, depth / 2.0f},
      {0, 0, 0, 1},
  };
  return m;
}

/**
 * @brief Get a matrix for transformation from world space to camera space
 *
 * @param camera_pos camera pos for calculating translation
 * @param lookat look-at direction for calculating roration
 * @param up up direction, unless u are operating a plane , this should be set
 * to y positive axis
 * @return Mat4f View Matrix of MVP
 */
static Mat4f view_trans(Vec3f camera_pos, Vec3f lookat, Vec3f up) noexcept {
  // accrooding to right-hand rule
  Vec3f l = lookat.normalize();
  Vec3f u = up.normalize();
  Vec3f r = (l ^ u).normalize();

  // firstly,simply move the model to set camera to origin
  Mat4f tr_t = {
      {1, 0, 0, -camera_pos.x},
      {0, 1, 0, -camera_pos.y},
      {0, 0, 1, -camera_pos.z},
      {0, 0, 0, 1},
  };
  Mat4f r_t = {
      {r.x, r.y, r.z, 0},
      {u.x, u.y, u.z, 0},
      {-l.x, -l.y, -l.z, 0},
      {0, 0, 0, 1},
  };
  return r_t * tr_t;
}

/**
 * @brief Get a matrix for transformation from local space to world space
 * here we simply set it to 4d-identity float type matrix
 *
 * @return Mat4f Model Matrix of MVP
 */
static Mat4f model_trans() noexcept {
  Mat4f m = Mat4f::identity();
  return m;
}

/**
 * @brief Get a matrix for transformation from view space to clip space, this
 * will include otrho trans and perspective trans
 *
 * @param eye_fov fov value
 * @param aspect_ratio width:height(16:9 always)
 * @param n near plate
 * @param f far plate
 * @return Mat4f Projection of MVP
 */
static Mat4f projection_trans(float eye_fov, float aspect_ratio, float n,
                              float f) noexcept {

  float t = tan(eye_fov / 2.0f * MY_PI / 180.0f) * n;
  float b = -t;
  float r = t * aspect_ratio;
  float l = -r;

  Mat4f persp = {
      {n, 0, 0, 0},
      {0, n, 0, 0},
      {0, 0, n + f, -n * f},
      {0, 0, 1, 0},
  };

  Mat4f ortho_s = {
      {2.0f / (r - l), 0, 0, 0},
      {0, 2.0f / (t - b), 0, 0},
      {0, 0, 2.0f / (n - f), 0},
      {0, 0, 0, 1},
  };

  Mat4f ortho_t = {
      {1, 0, 0, -(r + l) / 2.0f},
      {0, 1, 0, -(t + b) / 2.0f},
      {0, 0, 1, -(n + f) / 2.0f},
      {0, 0, 0, 1},
  };

  return ortho_s * ortho_t * persp;
}
#endif // __GUTILS_H__