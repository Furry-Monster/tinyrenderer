#include "tgaimage.h"
#include <cstdlib>
#include <iostream>
#include <utility>

void save_image(TGAImage &image);

// too slow!
void draw_line1(int x0, int y0, int x1, int y1, TGAImage &image,
                TGAColor color) {
  for (float t = 0.0f; t < 1.; t += 0.001f) {
    int x = x0 + (x1 - x0) * t;
    int y = y0 + (y1 - y0) * t;
    image.set_pixel(x, y, color);
  }
}

// only for not sleep and not inverse
void draw_line2(int x0, int y0, int x1, int y1, TGAImage &image,
                TGAColor color) {
  for (int x = x0; x <= x1; x++) {
    float t = (x - x0) / (float)(x1 - x0);
    int y = y0 * (1. - t) + y1 * t;
    image.set_pixel(x, y, color);
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
      image.set_pixel(y, x, color); // if transposed, de−transpose
    } else {
      image.set_pixel(x, y, color);
    }
  }
}

// optimized
void draw_line4(int x0, int y0, int x1, int y1, TGAImage &image,
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
  int dx = x1 - x0;
  int dy = y1 - y0;
  float derror = std::abs(dy / float(dx));
  float error = 0;
  int y = y0;
  for (int x = x0; x <= x1; x++) {
    if (steep) {
      image.set_pixel(y, x, color); // if transposed, de−transpose
    } else {
      image.set_pixel(x, y, color);
    }
    error += derror;
    if (error > .5) {
      y += (y1 > y0 ? 1 : -1);
      error -= 1.;
    }
  }
}

// optimized optimized !
void draw_line5(int x0, int y0, int x1, int y1, TGAImage &image,
                TGAColor color) {
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
      image.set_pixel(y, x, color);
    } else {
      image.set_pixel(x, y, color);
    }
    error2 += derror2;
    if (error2 > dx) {
      y += (y1 > y0 ? 1 : -1);
      error2 -= dx * 2;
    }
  }
}

int main() {
  TGAImage image(800, 800, TGAImage::RGB);
  TGAColor red(255, 0, 0, 255);
  TGAColor green(0, 255, 0, 255);
  TGAColor blue(0, 0, 255, 255);

  for (int i = 0; i < 100000; i++) {
    draw_line1(100, 100, 500, 700, image, red);
    draw_line2(200, 200, 600, 300, image, blue);
    draw_line3(0, 100, 100, 600, image, green);
    draw_line4(500, 100, 100, 500, image, red);
    draw_line5(500, 100, 100, 800, image, green);
  }
  image.flip_vertically();

  save_image(image);
  return 0;
}

void save_image(TGAImage &image) {
  std::string filename;
  std::cout << "Picture rendered succeessfully!\nWhere to save the result:\n";
  std::cin >> filename;
  if (filename.empty() || filename.length() <= 4 ||
      filename.substr(filename.length() - 4, 4) != ".tga") {
    filename.append(".tga");
  }
  image.write_tga_file(filename.c_str());
}