#ifndef __GMATH_H__
#define __GMATH_H__

#include <cmath>
#include <ostream>
#include <type_traits>

template <typename t> struct Vec2;
template <typename t> struct Vec3;

/**
 * @brief 2D Vector struct template
 *
 * @tparam T Type of vector value, must be arithmetic.
 */
template <typename T> struct Vec2 {
  static_assert(std::is_arithmetic<T>::value,
                "Vector type must be arithmetic!");

  union {
    struct {
      T u, v;
    };
    struct {
      T x, y;
    };
    T raw[2];
  };

  // constructors
  constexpr Vec2() noexcept : u(0), v(0) {}
  constexpr Vec2(T _u, T _v) noexcept : u(_u), v(_v) {}
  template <typename U>
  constexpr explicit Vec2(const Vec3<U> &other) noexcept
      : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

  // operator overrides.
  constexpr Vec2<T> operator+(const Vec2<T> &rhs) const noexcept {
    return Vec2<T>(u + rhs.u, v + rhs.v);
  }
  constexpr Vec2<T> operator-(const Vec2<T> &rhs) const noexcept {
    return Vec2<T>(u - rhs.u, v - rhs.v);
  }
  constexpr Vec2<T> operator*(T scalar) const noexcept {
    return Vec2<T>(u * scalar, v * scalar);
  }
  constexpr T operator[](int idx) const { return raw[idx]; }

  // vector ops
  T norm() const { return std::sqrt(x * x + y * y); }
  Vec2<T> &normalize(T l = 1) {
    T n = norm();
    if (n > 0) {
      T scale = l / n;
      u *= scale;
      v *= scale;
    }
    return *this;
  }

  // other ops.
  constexpr Vec3<T> toVec3() const noexcept { return Vec3<T>(x, y, 1); };

  template <typename U>
  friend std::ostream &operator<<(std::ostream &s, Vec2<T> &v);
};

/**
 * @brief 3D Vector struct template
 *
 * @tparam T Type of vector value, must be arithmetic.
 */
template <typename T> struct Vec3 {
  static_assert(std::is_arithmetic<T>::value,
                "Vector type must be arithmetic!");

  union {
    struct {
      T x, y, z;
    };
    struct {
      T ivert, iuv, inorm;
    };
    T raw[3];
  };

  // constructors
  constexpr Vec3() noexcept : x(0), y(0), z(0) {}
  constexpr Vec3(T _x, T _y, T _z) noexcept : x(_x), y(_y), z(_z) {}
  template <typename U>
  constexpr explicit Vec3(const Vec3<U> &other)
      : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)),
        z(static_cast<T>(other.z)) {}

  // operator overrides
  constexpr Vec3<T> operator^(const Vec3<T> &rhs) const noexcept {
    return Vec3<T>(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z,
                   x * rhs.y - y * rhs.x);
  }
  constexpr Vec3<T> operator+(const Vec3<T> &rhs) const noexcept {
    return Vec3<T>(x + rhs.x, y + rhs.y, z + rhs.z);
  }
  constexpr Vec3<T> operator-(const Vec3<T> &rhs) const noexcept {
    return Vec3<T>(x - rhs.x, y - rhs.y, z - rhs.z);
  }
  constexpr Vec3<T> operator*(T scalar) const noexcept {
    return Vec3<T>(x * scalar, y * scalar, z * scalar);
  }
  constexpr T operator*(const Vec3<T> &rhs) const noexcept {
    return x * rhs.x + y * rhs.y + z * rhs.z;
  }
  constexpr T operator[](size_t idx) const { return raw[idx]; }

  // vector ops.
  T norm() const { return std::sqrt(x * x + y * y + z * z); }
  Vec3<T> &normalize(T l = 1) {
    T n = norm();
    if (n > 0) {
      T scale = l / n;
      x *= scale;
      y *= scale;
      z *= scale;
    }
    return *this;
  }

  // other ops
  constexpr Vec2<T> toVec2() const noexcept { return Vec2<T>(x, y); }

  template <typename U>
  friend std::ostream &operator<<(std::ostream &s, Vec3<T> &v);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

template <typename T> std::ostream &operator<<(std::ostream &s, Vec2<T> &v) {
  s << "(" << v.x << ", " << v.y << ")\n";
  return s;
}

template <typename T> std::ostream &operator<<(std::ostream &s, Vec3<T> &v) {
  s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
  return s;
}

#endif // __GMATH_H__