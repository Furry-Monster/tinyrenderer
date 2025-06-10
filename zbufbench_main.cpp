#include "gmath.hpp"
#include "tgaimage.h"

const int width = 800;
const int height = 500;

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
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
      image.set(y, x, color);
    } else {
      image.set(x, y, color);
    }
  }
}

void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color,
               int ybuffer[]) {
  if (p0.x > p1.x) {
    std::swap(p0, p1);
  }
  for (int x = p0.x; x <= p1.x; x++) {
    float t = (x - p0.x) / (float)(p1.x - p0.x);
    int y = p0.y * (1. - t) + p1.y * t;
    if (ybuffer[x] < y) {
      ybuffer[x] = y;
      image.set(x, 0, color);
    }
  }
}

int main() {
  {
    TGAImage scene(800, 800, TGAImage::RGB);

    line(Vec2i(20, 34), Vec2i(744, 400), scene, red);
    line(Vec2i(120, 434), Vec2i(444, 400), scene, green);
    line(Vec2i(330, 463), Vec2i(594, 200), scene, blue);

    line(Vec2i(10, 10), Vec2i(790, 10), scene, white);

    scene.flip_vertically();
    scene.write_tga_file("scene.tga");
  }

  {
    TGAImage render(width, 16, TGAImage::RGB);
    int ybuffer[width];
    for (int i = 0; i < width; i++) {
      ybuffer[i] = std::numeric_limits<int>::min();
    }

    rasterize(Vec2i(20, 34), Vec2i(744, 400), render, red, ybuffer);
    rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer);
    rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue, ybuffer);

    render.flip_vertically();
    render.write_tga_file("render.tga");
  }

  return 0;
}