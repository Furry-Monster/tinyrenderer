#ifndef __GMATH_H__
#define __GMATH_H__

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <initializer_list>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

//------------------------ Vector Definitions ------------------------

template <typename T> struct Vec2;
template <typename T> struct Vec3;

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
  template <typename U>
  constexpr explicit Vec2(const Vec2<U> &other) noexcept
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
  friend std::ostream &operator<<(std::ostream &s, const Vec2<T> &v);
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
  constexpr explicit Vec3(const Vec2<U> &other)
      : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)),
        z(static_cast<T>(1.0f)) {}
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
  friend std::ostream &operator<<(std::ostream &s, const Vec3<T> &v);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

template <typename T>
std::ostream &operator<<(std::ostream &s, const Vec2<T> &v) {
  s << "(" << v.x << ", " << v.y << ")\n";
  return s;
}

template <typename T>
std::ostream &operator<<(std::ostream &s, const Vec3<T> &v) {
  s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
  return s;
}

//------------------------ Matrix Definitions ------------------------

template <typename T, int R, int C> class Matrix {
  static_assert(std::is_arithmetic<T>::value,
                "Matrix type must be arithmetic!");

private:
  std::vector<std::vector<T>> data;

public:
  // constructors
  constexpr Matrix() noexcept : data(R, std::vector<T>(C, 0)) {}
  constexpr Matrix(
      std::initializer_list<std::initializer_list<T>> init) noexcept {
    assert(init.size() == R && "Invalid number of rows");
    data.resize(R);
    for (int i = 0; i < R; i++) {
      assert(init.begin()[i].size() == C && "Invalid number of columns");
      data[i].resize(C);
      std::copy_n(init.begin()[i].begin(), C, data[i].begin());
    }
  }
  constexpr Matrix(const Matrix<T, R, C> &other) noexcept : data(other.data) {}

  // getters
  constexpr const T &operator()(int row, int col) const noexcept {
    assert(row >= 0 && row < R && col >= 0 && col < C &&
           "Matrix index out of bounds");
    return data[row][col];
  }
  constexpr T &operator()(int row, int col) noexcept {
    assert(row >= 0 && row < R && col >= 0 && col < C &&
           "Matrix index out of bounds");
    return data[row][col];
  }
  constexpr std::vector<T> &operator[](int row) noexcept {
    assert(row >= 0 && row < R && "Matrix row index out of bounds!");
    return data[row];
  }
  constexpr const std::vector<T> &operator[](int row) const noexcept {
    assert(row >= 0 && row < R && "Matrix row index out of bounds!");
    return data[row];
  }
  constexpr int rows() const noexcept { return R; }
  constexpr int cols() const noexcept { return C; }

  // calculations
  constexpr Matrix<T, R, C>
  operator+(const Matrix<T, R, C> &rhs) const noexcept {
    Matrix<T, R, C> result;
    for (int i = 0; i < R; i++) {
      for (int j = 0; j < C; j++) {
        result(i, j) = *this(i, j) + rhs(i, j);
      }
    }
    return result;
  }
  constexpr Matrix<T, R, C>
  operator-(const Matrix<T, R, C> &rhs) const noexcept {
    Matrix<T, R, C> result;
    for (int i = 0; i < R; i++) {
      for (int j = 0; j < C; j++) {
        result(i, j) = *this(i, j) - rhs(i, j);
      }
    }
    return result;
  }
  template <int N>
  constexpr Matrix<T, R, N>
  operator*(const Matrix<T, C, N> &rhs) const noexcept {
    Matrix<T, R, N> result;
    for (int i = 0; i < R; i++) {
      for (int j = 0; j < N; j++) {
        T sum = 0;
        for (int k = 0; k < C; k++) {
          sum += *this(i, k) * rhs(k, j);
        }
        result(i, j) = sum;
      }
    }
    return result;
  }
  constexpr Matrix<T, R, C> operator*(T scalar) const noexcept {
    Matrix<T, R, C> result;
    for (int i = 0; i < R; i++) {
      for (int j = 0; j < C; j++) {
        result(i, j) = *this(i, j) * scalar;
      }
    }
    return result;
  }
  constexpr Matrix<T, C, R> transpose() const noexcept {
    Matrix<T, C, R> result;
    for (int i = 0; i < R; i++) {
      for (int j = 0; j < C; j++) {
        result(j, i) = *this(i, j);
      }
    }
    return result;
  }
  constexpr Matrix<T, R, R> inverse() const {
    // well,for this method...
    // i'm not sure if it would do things well...
    static_assert(R == C, "Only square can be inverse!");

    // create augmented matrix [A|I]
    Matrix<T, R, 2 * R> aug;
    for (int i = 0; i < R; i++) {
      for (int j = 0; j < R; j++) {
        aug(i, j) = *this(i, j);
      }
      aug(i, i + R) = 1;
    }

    // G-J method (Gauss-Jordan Elimination)
    for (int i = 0; i < R; i++) {
      // find main
      T pivot = aug(i, i);
      if (std::abs(pivot) < 1e-6) {
        throw std::runtime_error("Matrix is not invertible");
      }

      // normalize along diag
      for (int j = 0; j < 2 * R; j++) {
        aug(i, j) /= pivot;
      }

      // eliminating
      for (int k = 0; k < R; k++) {
        if (k != i) {
          T factor = aug(k, i);
          for (int j = 0; j < 2 * R; j++) {
            aug(k, j) -= factor * aug(i, j);
          }
        }
      }
    }

    // load inverse matrix
    Matrix<T, R, R> result;
    for (int i = 0; i < R; i++) {
      for (int j = 0; j < R; j++) {
        result(i, j) = aug(i, j + R);
      }
    }

    return result;
  }

  // other ops
  constexpr bool isZero(T epsilon = 1e-6) const noexcept {
    for (int i = 0; i < R; i++) {
      for (int j = 0; j < C; j++) {
        if (std::abs((*this)(i, j)) > epsilon) {
          return false;
        }
      }
    }
    return true;
  };
  constexpr bool isSymmetric(T epsilon = 1e-6) const noexcept {
    static_assert(R == C, "Symmetry check only valid for square matrices!");

    for (int i = 0; i < R; i++) {
      for (int j = i + 1; j < C; j++) {
        if (std::abs((*this)(i, j) - (*this)(j, i)) > epsilon) {
          return false;
        }
      }
    }
    return true;
  };

  constexpr static Matrix<T, R, R> identity() noexcept {
    static_assert(R == C, "Identity matrix must be square!");

    Matrix<T, R, R> result;
    for (int i = 0; i < R; i++)
      result(i, i) = 1;
    return result;
  }

  constexpr static Matrix<T, R, C> zeros() noexcept {
    return Matrix<T, R, C>();
  }

  constexpr static Matrix<T, R, C> ones() noexcept {
    Matrix<T, R, C> result;
    for (int i = 0; i < R; i++)
      for (int j = 0; j < C; j++)
        result(i, j) = T(1);
    return result;
  }

  template <typename U>
  friend std::ostream &operator<<(std::ostream &s, const Matrix<T, R, C> &mat) {
    for (int i = 0; i < R; i++) {
      s << "[";
      for (int j = 0; j < C; j++) {
        s << mat(i, j);
        if (j < C - 1)
          s << ", ";
      }
      s << "]\n";
    }
    return s;
  }
};

typedef Matrix<float, 4, 4> Mat4f;
typedef Matrix<float, 3, 3> Mat3f;
typedef Matrix<float, 2, 2> Mat2f;

#endif // __GMATH_H__