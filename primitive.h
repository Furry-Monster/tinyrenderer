#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include "gmath.h"
#include "renderer.h"
#include "tgaimage.h"
#include <vector>

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
  Vec3f verts[3];
  Vec3f rverts_[3];
  Vec2f uvs_[3];
  Vec3f normals_[3];

  unsigned int shading_mode_;

public:
  explicit Triangle(unsigned int mode) noexcept : shading_mode_(mode) {}

  void set_verts(Vec3f *verts) {
    for (int i = 0; i < 3; i++)
      verts[i] = verts[i];
  }
  void set_rverts(Vec3f *rverts) {
    for (int i = 0; i < 3; i++)
      rverts_[i] = rverts[i];
  }
  void set_uvs(Vec2f *uvs) {
    for (int i = 0; i < 3; i++)
      uvs_[i] = uvs[i];
  }
  void set_normals(Vec3f *normals) {
    for (int i = 0; i < 3; i++)
      normals_[i] = normals[i];
  }
  void set_shading_mode(unsigned int mode) { shading_mode_ = mode; }

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

  /**
   * @brief Triangle drawing function from
   * trianglebench_main.cpp:draw_triangle4()
   *
   * @param image image to draw triangle
   * @param zbuf zbuf for depth testing
   */
  void draw(TGAImage &image, float *zbuf) noexcept override {
    int xmax = -1, ymax = -1;
    int xmin = 8000,
        ymin = 8000; // i dont think somebody would use 8k screen...
    Vec2i rverts_int[3];

    for (int i = 0; i < 3; i++) {
      Vec2i cur = Vec2i(rverts_[i]);
      rverts_int[i] = cur;

      if (cur.x < xmin)
        xmin = cur.x;
      if (cur.x > xmax)
        xmax = cur.x;
      if (cur.y < ymin)
        ymin = cur.y;
      if (cur.y > ymax)
        ymax = cur.y;
    }

    Vec3f pixelPos, bc;
    for (int i = xmin; i < xmax; i++) {
      for (int j = ymin; j < ymax; j++) {
        pixelPos.x = i;
        pixelPos.y = j;
        pixelPos.z = 0.0f;
        bc = calc_barycentric(rverts_int, pixelPos.toVec2());
        // Do barycentric test then.
        // If barycentric have any negative value,
        // the pixelPos would be regard to be outlined,
        // although it's inside bounding box
        if (bc.x < 0 || bc.y < 0 || bc.z < 0)
          continue;
        // depth buffer testing here.
        pixelPos.z =
            rverts_[0].z * bc.x + rverts_[1].z * bc.y + rverts_[2].z * bc.z;
        if (zbuf[int(pixelPos.x + pixelPos.y * image.get_width())] <
            pixelPos.z) {
          zbuf[int(pixelPos.x + pixelPos.y * image.get_width())] = pixelPos.z;
          // if only we update buffer , the "frame buffer" would be
          // update (actually we consider the image reference as our frame
          // buffer XD )
          image.set(i, j, white);
        }
      }
    }
  }

  /**
   * @brief Drawing triangle piece and texturing
   *
   * @param image image to draw
   * @param zbuf zbuffer reference for depth testing
   * @param intensity light intensity, so simple :-(
   */
  void draw(TGAImage &image, float *zbuf, TGAImage &diffuse,
            float intensity) noexcept {
    int xmax = -1, ymax = -1;
    int xmin = 8000,
        ymin = 8000; // i dont think somebody would use 8k screen...
    Vec2i vertices_2i[3];
    TGAColor color;

    for (int i = 0; i < 3; i++) {
      Vec2i cur_vertex = Vec2i(rverts_[i]);
      vertices_2i[i] = cur_vertex;

      if (cur_vertex.x < xmin)
        xmin = cur_vertex.x;
      if (cur_vertex.x > xmax)
        xmax = cur_vertex.x;
      if (cur_vertex.y < ymin)
        ymin = cur_vertex.y;
      if (cur_vertex.y > ymax)
        ymax = cur_vertex.y;
    }

    Vec3f pixelPos, bc;
    for (int i = xmin; i < xmax; i++) {
      for (int j = ymin; j < ymax; j++) {
        pixelPos.x = i;
        pixelPos.y = j;
        pixelPos.z = 0.0f;

        bc = calc_barycentric(vertices_2i, pixelPos.toVec2());
        // Do barycentric test then.
        // If barycentric have any negative value,
        // the pixelPos would be regard to be outlined,
        // although it's inside bounding box
        if (bc.x < 0 || bc.y < 0 || bc.z < 0)
          continue;

        // barycentric interpolate texturing
        if ((shading_mode_ & 0x1) != 0) {
          // &0x1 for diffuse bit
          Vec2f tex_pos(0, 0);
          for (int k = 0; k < 3; k++) {
            tex_pos.x += uvs_[k].u * bc[k];
            tex_pos.y += uvs_[k].v * bc[k];
          }
          int sample_x = tex_pos.u * diffuse.get_width();
          int sample_y = tex_pos.v * diffuse.get_height();

          color = diffuse.get(sample_x, sample_y);
        } else {
          // use no texture maps...
          // we just use white/gray to render it.
          color = white;
        }

        // an easy lighting,well, it's not enough for me...
        color = TGAColor(color.r * intensity, color.g * intensity,
                         color.b * intensity, color.a);

        // depth buffer testing here.
        for (int k = 0; k < 3; k++)
          pixelPos.z += rverts_[k].z * bc[k];
        if (zbuf[int(pixelPos.x + pixelPos.y * image.get_width())] <
            pixelPos.z) {
          zbuf[int(pixelPos.x + pixelPos.y * image.get_width())] = pixelPos.z;
          // if only we update buffer , the "frame buffer" would be
          // update (actually we consider the image reference as our frame
          // buffer XD )
          image.set(i, j, color);
        }
      }
    }
  }
};

#endif // __PRIMITIVE_H__