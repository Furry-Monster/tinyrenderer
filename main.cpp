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
};

void print_usage() {
  std::cout
      << "Usage: tinyrenderer [Options] <filepath>\n"
      << "Options:\n"
      << "  -m, --mode     RenderMode (line/triangle, 默认: line)\n"
      << "  -w, --width    Width for output image (默认: 800)\n"
      << "  -h, --height   Height for output image (默认: 800)\n"
      << "  -o, --output   Filename for output image (默认: output.tga)\n"
      << "  --help         Show help message\n"
      << "Examples:\n"
      << "  tinyrenderer -m triangle obj/african_head.obj\n"
      << "  tinyrenderer --mode line --width 1024 --height 1024 model.obj\n";
}

/**
 * @brief Parse arguments for main, load them into RenderOption structure
 *
 * @param argc As defined in main()
 * @param argv As defined in main()
 * @return RenderOptions Option structure for rendering configurations
 */
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
          std::cerr << "Error: Invalid rendering mode " << mode << std::endl;
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
Vec3f barycentric2d(Vec2i *pts, Vec2f P) {
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

void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color,
               int *zbuffer) {
  if (p0.x > p1.x) {
    std::swap(p0, p1);
  }
  for (int x = p0.x; x <= p1.x; x++) {
    float t = (x - p0.x) / (float)(p1.x - p0.x);
    int y = p0.y * (1. - t) + p1.y * t;
    if (zbuffer[x] < y) {
      zbuffer[x] = y;
      image.set(x, 0, color);
    }
  }
}

/**
 * @brief Triangle drawing function from trianglebench_main.cpp:draw_triangle4()
 *
 * @param pts point set
 * @param zbuf zbuffer reference for depth testing
 * @param image image to draw
 * @param color color for image (without interpolate)
 */
void draw_triangle(Vec2i *pts, float *zbuf, TGAImage &image, TGAColor color) {
  int xmax = -1, ymax = -1;
  int xmin = 8000, ymin = 8000; // i dont think somebody would use 8k screen...

  for (int i = 0; i < 3; i++) {
    Vec2i cur = pts[i];
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
      bc = barycentric2d(pts, Vec2f(pixelPos.x, pixelPos.y));
      if (bc.x < 0 || bc.y < 0 || bc.z < 0)
        continue;
      else
        image.set(i, j, color);
    }
  }
}

int main(int argc, char **argv) {
  RenderOptions options = parse_args(argc, argv);

  TGAImage image(options.width, options.height, TGAImage::RGB);
  Model *model = new Model(options.filepath.c_str());
  if (model == nullptr) {
    std::cerr << "Error: Can't load model from " << options.filepath
              << std::endl;
    return 1;
  }
  float *zbuf = new float[options.width * options.height];

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
            screen_coords, zbuf, image,
            TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
      }
    }
  }

  image.flip_vertically();
  image.write_tga_file(options.outputpath.c_str());
  delete model;

  return 0;
}