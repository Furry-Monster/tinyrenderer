#include "tgaimage.h"

/**
 * @brief This is the draw_line5 function from linebench_main.cpp
 *
 * @param x0 x value of point 0
 * @param y0 y value of point 0
 * @param x1 x value of point 1
 * @param y1 y value of point 1
 * @param image image to be rendered (where line will draw on)
 * @param color color of the line
 */
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
  bool steep = false;
  if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
    std::swap(x0, y0);
    std::swap(x1, y1);
    steep = true;
  }
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }
  int dx = x1 - x0;
  int dy = y1 - y0;
  int derror2 = std::abs(dy) * 2;
  int error2 = 0;
  int y = y0;
  for (int x = x0; x <= x1; x++) {
    if (steep) {
      image.set(y, x, color);
    } else {
      image.set(x, y, color);
    }
    error2 += derror2;
    if (error2 > dx) {
      y += (y1 > y0 ? 1 : -1);
      error2 -= dx * 2;
    }
  }
}

int main(int argc, char **argv) {
  TGAImage image(800, 800, TGAImage::RGB);
  TGAColor red(255, 0, 0, 255);

  return 0;
}