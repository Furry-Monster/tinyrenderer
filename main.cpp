#include "tgaimage.h"
#include <iostream>

void save_image(const TGAImage &image);

void draw_line(int x0, int y0, int x1, int y1, TGAImage &image,
               TGAColor color) {
  for (float t = 0; t < 1; t += 0.1) {
    int x = x0 + (x1 - x0) * t;
    int y = y0 + (y1 - y0) * t;
    image.set(x, y, color);
  }
}

int main(int argc, char **argv) {
  TGAImage image(800, 800, TGAImage::RGB);
  TGAColor red(255, 0, 0, 255);

  draw_line(100, 100, 500, 700, image, red);

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