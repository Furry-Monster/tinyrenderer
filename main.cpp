#include "tgaimage.h"
#include <cstdlib>
#include <iostream>
#include <utility>

void save_image(const TGAImage &image);

// too slow !
void draw_line1(int x0, int y0, int x1, int y1, TGAImage &image,
                TGAColor color) {
  for (float t = 0.0f; t < 1.; t += 0.001f) {
    int x = x0 + (x1 - x0) * t;
    int y = y0 + (y1 - y0) * t;
    image.set(x, y, color);
  }
}

// only for not sleep and not inverse
void draw_line2(int x0, int y0, int x1, int y1, TGAImage &image,
                TGAColor color) {
  for (int x = x0; x <= x1; x++) {
    float t = (x - x0) / (float)(x1 - x0);
    int y = y0 * (1. - t) + y1 * t;
    image.set(x, y, color);
  }
}

// best fit
void draw_line3(int x0, int y0, int x1, int y1, TGAImage &image,
                TGAColor color) {
  bool steep = false;
  if (std::abs(x0 - x1) <
      std::abs(y0 - y1)) { // if the line is steep, we transpose the image
    std::swap(x0, y0);
    std::swap(x1, y1);
    steep = true;
  }
  if (x0 > x1) { // make it left−to−right
    std::swap(x0, x1);
    std::swap(y0, y1);
  }
  for (int x = x0; x <= x1; x++) {
    float t = (x - x0) / (float)(x1 - x0);
    int y = y0 * (1. - t) + y1 * t;
    if (steep) {
      image.set(y, x, color); // if transposed, de−transpose
    } else {
      image.set(x, y, color);
    }
  }
}

int main(int argc, char **argv) {
  TGAImage image(800, 800, TGAImage::RGB);
  TGAColor red(255, 0, 0, 255);
  TGAColor green(0, 255, 0, 255);
  TGAColor blue(0, 0, 255, 255);

  draw_line1(100, 100, 500, 700, image, red);
  draw_line2(200, 200, 600, 300, image, blue);
  draw_line3(0, 100, 100, 600, image, green);
  draw_line3(500, 100, 100, 500, image, red);

  image.flip_vertically();

  save_image(image);
  return 0;
}

void save_image(const TGAImage &image) {
  std::string filename;
  std::cout << "Picture rendered succeessfully!\nWhere to save the result:\n";
  std::cin >> filename;
  if (filename.empty() || filename.length() <= 4 ||
      filename.substr(filename.length() - 4, 4) != ".tga") {
    filename.append(".tga");
  }
  image.write_tga_file(filename.c_str());
}