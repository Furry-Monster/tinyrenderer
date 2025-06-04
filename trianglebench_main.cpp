#include "gmath.h"
#include "tgaimage.h"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <utility>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 800;
const int height = 800;

void save_image(TGAImage &image);

void draw_line(Vec2i v0, Vec2i v1, TGAImage &image, TGAColor color) {
  bool steep = false;
  if (std::abs(v0.x - v1.x) < std::abs(v0.y - v1.y)) {
    std::swap(v0.x, v0.y);
    std::swap(v1.x, v1.y);
    steep = true;
  }
  if (v0.x > v1.x) {
    std::swap(v0.x, v1.x);
    std::swap(v0.y, v1.y);
  }
  int dx = v1.x - v0.x;
  int dy = v1.y - v0.y;
  int derror2 = std::abs(dy) * 2;
  int error2 = 0;
  int y = v0.y;
  for (int x = v0.x; x <= v1.x; x++) {
    if (steep) {
      image.set(y, x, color);
    } else {
      image.set(x, y, color);
    }
    error2 += derror2;
    if (error2 > dx) {
      y += (v1.y > v0.y ? 1 : -1);
      error2 -= dx * 2;
    }
  }
}

// this method draw a triangle line frame
void draw_triangle1(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image,
                    TGAColor color) {
  draw_line(v0, v1, image, color);
  draw_line(v1, v2, image, color);
  draw_line(v2, v0, image, color);
}

void draw_triangle2(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image,
                    TGAColor color) {
  if (v0.y > v1.y)
    std::swap(v0, v1);
  if (v0.y > v2.y)
    std::swap(v0, v2);
  if (v1.y > v2.y)
    std::swap(v1, v2);

  Vec2i dir0 = v0 - v2;
  Vec2i dir1 = v1 - v2;
  for (float i = 0; i < 1.0f; i += 0.001f) {
    Vec2i v_left = v2 + dir0 * i;
    Vec2i v_right = v2 + dir1 * i;
    draw_line(v_left, v_right, image, color);
  }
}

// this method draw a solid triangle
void draw_triangle3(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image,
                    TGAColor color) {
  // To keep it simple, the triangle won't use interpolate color:
  // 1. Sort vertices of the triangle by their y-coordinates;
  // 2. Rasterize simultaneously the left and the right sides of the triangle;
  // 3. Draw a horizontal line segment between the left and the right boundary
  // points.
  // sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!)
  if (v0.y > v1.y)
    std::swap(v0, v1);
  if (v0.y > v2.y)
    std::swap(v0, v2);
  if (v1.y > v2.y)
    std::swap(v1, v2);
  int total_height = v2.y - v0.y;

  for (int i = 0; i < total_height; i++) {
    bool second_half = i > v1.y - v0.y || v1.y == v0.y;
    int segment_height = second_half ? v2.y - v1.y : v1.y - v0.y;
    float alpha = (float)i / total_height;
    float beta = (float)(i - (second_half ? v1.y - v0.y : 0)) /
                 segment_height; // be careful: with above conditions no
                                 // division by zero here
    Vec2i A = v0 + (v2 - v0) * alpha;
    Vec2i B = second_half ? v1 + (v2 - v1) * beta : v0 + (v1 - v0) * beta;
    if (A.x > B.x)
      std::swap(A, B);
    for (int j = A.x; j <= B.x; j++) {
      image.set(j, v0.y + i,
                color); // attention, due to int casts v0.y+i != A.y
    }
  }
}

int main(int argc, char **argv) {
  TGAImage image(width, height, TGAImage::RGB);

  Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
  Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
  Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};

  for (int i = 0; i < 10000; i++) {
    draw_triangle1(t0[0], t0[1], t0[2], image, red);
    draw_triangle2(t1[0], t1[1], t1[2], image, white);
    draw_triangle3(t2[0], t2[1], t2[2], image, green);
    std::cout << "Work on no." << i << " rendering...\n";
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