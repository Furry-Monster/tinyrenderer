#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include "gmath.h"
#include "tgaimage.h"

class Primitive {
public:
  virtual void draw(TGAImage &image, float *zbuf) noexcept = 0;
};

struct Line : public Primitive {
private:
  Vec2i start_, end_;
  TGAColor color_;

public:
  Line() {}
  Line(TGAColor color) : color_(color) {}
  Line(Vec2i start, Vec2i end, TGAColor color)
      : start_(start), end_(end), color_(color) {}

  void set_point(Vec2i start, Vec2i end) {
    start_ = start;
    end_ = end;
  };
  void set_color(TGAColor color) { color_ = color; };

  /**
   * @brief Drawing a line, copy from linebenchmark.cpp:draw_line5();
   *
   * @param image image to draw the line
   * @param zbuf umm...it's useless...
   */
  void draw(TGAImage &image, float *zbuf) noexcept override {
    bool steep = false;
    if (std::abs(start_.x - end_.x) < std::abs(start_.y - end_.y)) {
      std::swap(start_.x, start_.y);
      std::swap(end_.x, end_.y);
      steep = true;
    }
    if (start_.x > end_.x) {
      std::swap(start_.x, end_.x);
      std::swap(start_.y, end_.y);
    }
    int dx = end_.x - start_.x;
    int dy = end_.y - start_.y;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = start_.y;
    for (int x = start_.x; x <= end_.x; x++) {
      if (steep) {
        image.set(y, x, color_);
      } else {
        image.set(x, y, color_);
      }
      error2 += derror2;
      if (error2 > dx) {
        y += (end_.y > start_.y ? 1 : -1);
        error2 -= dx * 2;
      }
    }
  }
};

struct Triangle : public Primitive {
private:
  Vec3f vertices_[3];
  Vec2f uvs_[3];
  TGAColor color_;

public:
  Triangle() {}
  Triangle(TGAColor color) : color_(color) {}

  void set_color(TGAColor color) { color_ = color; }

  /**
   * @brief Find 2d coord's barycentric.
   *
   * @param pts triangle set
   * @param P P for finding barycentric
   * @return Vec3f uv_map , structure {1,y,x} , be careful it's inverse :-)
   */
  Vec3f calc_barycentric(Vec2i *pts, Vec2f P) noexcept {
    Vec3f x_vec =
        Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x);
    Vec3f y_vec =
        Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
    // cross multiply here get the solution for linear problem:
    // u * AB_vec+ v * AC_vec + PA_vec = 0_vec
    // we can depart it along axis x & y :
    // [u * AB_vec_x + v * AC_vec_x + PA_vac_x = 0]
    // [u * AB_vec_y + v * AC_vec_y + PA_vac_y = 0]
    Vec3f uv = x_vec ^ y_vec;
    if (std::abs(uv.z) < 1)
      // triangle is degenerate, in this case return something with negative
      // coordinates
      return Vec3f(-1.0f, 1.0f, 1.0f);
    else
      return Vec3f(1.0f - (uv.x + uv.y) / uv.z, uv.y / uv.z,
                   uv.x / uv.z); // return normalized uv result.
  }

  void draw(TGAImage &image, float *zbuf) noexcept override {}
};

#endif // __PRIMITIVE_H__