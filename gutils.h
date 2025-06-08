#ifndef __GUTILS_H__
#define __GUTILS_H__

#include "gmath.h"

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

#endif // __GUTILS_H__