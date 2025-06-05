#include "gmath.h"
#include "model.h"
#include "tgaimage.h"
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

enum RenderingMode {
  LINE,
  TRIANGLE,
};

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

struct RenderOptions {
  RenderingMode mode = RenderingMode::LINE;
  std::string filepath = "obj/african_head.obj";
  std::string outputpath = "output.tga";
  int width = 800;
  int height = 800;
} options;

void print_usage() {
  std::cout
      << "使用方法: tinyrenderer [选项] <文件路径>\n"
      << "选项:\n"
      << "  -m, --mode     渲染模式 (line/triangle, 默认: line)\n"
      << "  -w, --width    输出图像宽度 (默认: 800)\n"
      << "  -h, --height   输出图像高度 (默认: 800)\n"
      << "  -o, --output   设置输出图像名称 (默认: output.tga)\n"
      << "  --help         显示此帮助信息\n"
      << "示例:\n"
      << "  tinyrenderer -m triangle obj/african_head.obj\n"
      << "  tinyrenderer --mode line --width 1024 --height 1024 model.obj\n";
}

RenderOptions parse_args(int argc, char **argv) {
  RenderOptions options;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--help") {
      print_usage();
      exit(0);
    } else if (arg == "-m" || arg == "--mode") {
      if (i + 1 < argc) {
        std::string mode = argv[++i];
        if (mode == "line") {
          options.mode = RenderingMode::LINE;
        } else if (mode == "triangle") {
          options.mode = RenderingMode::TRIANGLE;
        } else {
          std::cerr << "错误: 无效的渲染模式 " << mode << std::endl;
          exit(1);
        }
      }
    } else if (arg == "-w" || arg == "--width") {
      if (i + 1 < argc) {
        options.width = std::stoi(argv[++i]);
      }
    } else if (arg == "-h" || arg == "--height") {
      if (i + 1 < argc) {
        options.height = std::stoi(argv[++i]);
      }
    } else if (arg == "-o" || arg == "--output") {
      if (i + 1 < argc) {
        options.outputpath = argv[++i];
      }
    } else if (arg[0] != '-') {
      options.filepath = arg;
    }
  }
  return options;
}

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
void draw_line(int x0, int y0, int x1, int y1, TGAImage &image,
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

/**
 * @brief Find 2d coord's barycentric.
 *
 * @param pts triangle set
 * @param P P for finding barycentric
 * @return Vec3f uv_map , structure {1,y,x} , be careful it's inverse
 */
Vec3f barycentric2d(Vec2i *pts, Vec2i P) {
  Vec3f x_vec = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x);
  Vec3f y_vec = Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
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
 * @brief Triangle drawing function from trianglebench_main.cpp:draw_triangle4()
 *
 * @param pts point set
 * @param image image to draw
 * @param color color for image (without interpolate)
 */
void draw_triangle(Vec2i *pts, TGAImage &image, TGAColor color) {
  int xmax = -1, ymax = -1;
  int xmin = 8000, ymin = 8000; // i dont think somebody would use 8k screen...
  Vec2i pixelPos, cur;

  for (int i = 0; i < 3; i++) {
    cur = pts[i];
    if (cur.x < xmin)
      xmin = cur.x;
    if (cur.x > xmax)
      xmax = cur.x;
    if (cur.y < ymin)
      ymin = cur.y;
    if (cur.y > ymax)
      ymax = cur.y;
  }
  for (int i = xmin; i < xmax; i++) {
    for (int j = ymin; j < ymax; j++) {
      pixelPos.x = i;
      pixelPos.y = j;
      Vec3f bc = barycentric2d(pts, pixelPos);
      if (bc.x < 0 || bc.y < 0 || bc.z < 0)
        continue;
      else
        image.set(i, j, color);
    }
  }
}

int main(int argc, char **argv) {
  options = parse_args(argc, argv);

  TGAImage image(options.width, options.height, TGAImage::RGB);
  Model *model = new Model(options.filepath.c_str());
  if (model == nullptr) {
    std::cerr << "错误: 无法加载模型文件: " << options.filepath << std::endl;
    return 1;
  }

  if (options.mode == RenderingMode::LINE) {
    for (int i = 0; i < model->nfaces(); i++) {
      std::vector<int> face = model->face(i);
      for (int j = 0; j < 3; j++) {
        Vec3f v0 = model->vert(face[j]);
        Vec3f v1 = model->vert(face[(j + 1) % 3]);
        int x0 = (v0.x + 1.) * options.width / 2.;
        int y0 = (v0.y + 1.) * options.height / 2.;
        int x1 = (v1.x + 1.) * options.width / 2.;
        int y1 = (v1.y + 1.) * options.height / 2.;
        draw_line(x0, y0, x1, y1, image, white);
      }
    }
  } else if (options.mode == RenderingMode::TRIANGLE) {
    Vec3f light_dir(0, 0, -1); // define light_dir
    for (int i = 0; i < model->nfaces(); i++) {
      std::vector<int> face = model->face(i);
      Vec2i screen_coords[3];
      Vec3f world_coords[3];
      for (int j = 0; j < 3; j++) {
        Vec3f v = model->vert(face[j]);
        screen_coords[j] = Vec2i((v.x + 1.) * options.width / 2.,
                                 (v.y + 1.) * options.height / 2.);
        world_coords[j] = v;
      }
      Vec3f n = (world_coords[2] - world_coords[0]) ^
                (world_coords[1] - world_coords[0]);
      n.normalize();
      float intensity = n * light_dir;
      if (intensity > 0) {
        draw_triangle(
            screen_coords, image,
            TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
      }
    }
  }

  image.flip_vertically();
  image.write_tga_file(options.outputpath.c_str());
  delete model;

  return 0;
}