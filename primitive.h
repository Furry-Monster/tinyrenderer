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
  Vec2f uvs_[2];
  TGAColor color_;

public:
  Triangle() {}
  Triangle(TGAColor color) : color_(color) {}

  void draw(TGAImage &image, float *zbuf) noexcept override {}
};

#endif // __PRIMITIVE_H__