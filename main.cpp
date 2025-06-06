#include "gmath.h"
#include "model.h"
#include "tgaimage.h"
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

enum RenderingMode {
  LINE,
  TRIANGLE,
  TEXTURED,
};

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

struct RenderOptions {
  RenderingMode mode = RenderingMode::LINE;

  std::string objpath = "obj/african_head.obj";
  std::string texpath = "texture/african_head_diffuse.tga";
  std::string outputpath = "output.tga";

  int width = 800;
  int height = 800;
};

void print_usage() {
  std::cout
      << "Usage: tinyrenderer [Options] <filepath>\n"
      << "Options:\n"
      << "  -m, --mode     RenderMode (line/triangle/textured, 默认: line)\n"
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
const RenderOptions parse_args(int argc, char **argv) {
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
        } else if (mode == "textured") {
          options.mode = RenderingMode::TEXTURED;
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
      options.objpath = arg;
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
 * @return Vec3f uv_map , structure {1,y,x} , be careful it's inverse :-)
 */
Vec3f barycentric2d(Vec2i *pts, Vec2f P) {
  Vec3f x_vec = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x);
  Vec3f y_vec = Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
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

// void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color,
//  int *zbuffer) {
// if (p0.x > p1.x) {
// std::swap(p0, p1);
// }
// for (int x = p0.x; x <= p1.x; x++) {
// float t = (x - p0.x) / (float)(p1.x - p0.x);
// int y = p0.y * (1. - t) + p1.y * t;
// if (zbuffer[x] < y) {
// zbuffer[x] = y;
// image.set(x, 0, color);
// }
// }
// }

/**
 * @brief Triangle drawing function from trianglebench_main.cpp:draw_triangle4()
 *
 * @param pts point set
 * @param zbuf zbuffer reference for depth testing
 * @param image image to draw
 * @param color color for image (without interpolate)
 */
void draw_triangle(Vec3f *pts, float *zbuf, TGAImage &image, TGAColor color) {
  int xmax = -1, ymax = -1;
  int xmin = 8000, ymin = 8000; // i dont think somebody would use 8k screen...
  Vec2i pts_i[3];

  for (int i = 0; i < 3; i++) {
    Vec2i cur = Vec2i(pts[i]);
    pts_i[i] = cur;

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
      bc = barycentric2d(pts_i, pixelPos.toVec2());
      // Do barycentric test then.
      // If barycentric have any negative value,
      // the pixelPos would be regard to be outlined,
      // although it's inside bounding box
      if (bc.x < 0 || bc.y < 0 || bc.z < 0)
        continue;
      // depth buffer testing here.
      pixelPos.z = pts[0].z * bc.x + pts[1].z * bc.y + pts[2].z * bc.z;
      if (zbuf[int(pixelPos.x + pixelPos.y * image.get_width())] < pixelPos.z) {
        zbuf[int(pixelPos.x + pixelPos.y * image.get_width())] = pixelPos.z;
        // if only we update buffer , the "frame buffer" would be
        // update (actually we consider the image reference as our frame buffer
        // XD )
        image.set(i, j, color);
      }
    }
  }
}

/**
 * @brief Drawing triangle piece and texturing
 *
 * @param screen_coords screen coords for locating pieces
 * @param tex_coords texture coords for locating texture
 * @param zbuf zbuffer reference for depth testing
 * @param image image to draw
 * @param texture textrue image reference
 * @param intensity light intensity, so simple :-(
 */
void draw_triangle(Vec3f *screen_coords, Vec2f *tex_coords, float *zbuf,
                   TGAImage &image, const TGAImage &texture, float intensity) {
  int xmax = -1, ymax = -1;
  int xmin = 8000, ymin = 8000; // i dont think somebody would use 8k screen...
  Vec2i pts_i[3];

  for (int i = 0; i < 3; i++) {
    Vec2i cur = Vec2i(screen_coords[i]);
    pts_i[i] = cur;

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
      bc = barycentric2d(pts_i, pixelPos.toVec2());
      // Do barycentric test then.
      // If barycentric have any negative value,
      // the pixelPos would be regard to be outlined,
      // although it's inside bounding box
      if (bc.x < 0 || bc.y < 0 || bc.z < 0)
        continue;

      // barycentric interpolate texturing
      Vec2f tex_pos(0, 0);
      tex_pos.x = tex_coords[0].x * bc.x + tex_coords[1].x * bc.y +
                  tex_coords[2].x * bc.z;
      tex_pos.y = tex_coords[0].y * bc.x + tex_coords[1].y * bc.y +
                  tex_coords[2].y * bc.z;
      int tex_x = tex_pos.x * texture.get_width();
      int tex_y = tex_pos.y * texture.get_height();
      TGAColor color = texture.get(tex_x, tex_y);
      color = TGAColor(color.r * intensity, color.g * intensity,
                       color.b * intensity, color.a);

      // depth buffer testing here.
      pixelPos.z = screen_coords[0].z * bc.x + screen_coords[1].z * bc.y +
                   screen_coords[2].z * bc.z;
      if (zbuf[int(pixelPos.x + pixelPos.y * image.get_width())] < pixelPos.z) {
        zbuf[int(pixelPos.x + pixelPos.y * image.get_width())] = pixelPos.z;
        // if only we update buffer , the "frame buffer" would be
        // update (actually we consider the image reference as our frame buffer
        // XD )
        image.set(i, j, color);
      }
    }
  }
}

/**
 * @brief Transform world position to screen position, keep z value
 *
 * @param v vertex transform in world coord.
 * @param width viewport width.
 * @param height viewport height.
 * @return Vec3f vertex transform in screen coord.
 */
Vec3f world2screen(Vec3f v, int width, int height) {
  return Vec3f(int((v.x + 1.) * width / 2. + .5),
               int((v.y + 1.) * height / 2. + .5), v.z);
}

int main(int argc, char **argv) {
  const RenderOptions options = parse_args(argc, argv);

  TGAImage image(options.width, options.height, TGAImage::RGB);
  Model *model = new Model(options.objpath.c_str());
  if (model == nullptr) {
    std::cerr << "Error: Can't load model from " << options.objpath
              << std::endl;
    return 1;
  }
  TGAImage texture;
  if (!texture.read_tga_file(options.texpath.c_str())) {
    std::cerr << "Error: Can't load texture from " << options.texpath
              << std::endl;
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
    // allocate depth buffer.
    float *zbuf = new float[options.width * options.height];
    for (int i = 0; i < options.width * options.height; i++)
      zbuf[i] = -std::numeric_limits<float>::max();

    Vec3f light_dir(0, 0, -1); // define light_dir
    for (int i = 0; i < model->nfaces(); i++) {
      // load data from model
      std::vector<int> face = model->face(i);
      Vec3f screen_coords[3]; // coord of 3 verts trace on screen plate
      Vec3f world_coords[3];  // coord of 3 verts without any transform
      for (int j = 0; j < 3; j++) {
        Vec3f v = model->vert(face[j]);
        world_coords[j] = v;
        screen_coords[j] = world2screen(v, options.width, options.height);
      }

      // calculate light intensity
      Vec3f n = (world_coords[2] - world_coords[0]) ^
                (world_coords[1] - world_coords[0]);
      n.normalize();
      float intensity = n * light_dir;

      // render on image, triangle as piece
      if (intensity > 0) {
        draw_triangle(
            screen_coords, zbuf, image,
            TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
      }
    }
  } else if (options.mode == RenderingMode::TEXTURED) {
    // allocate depth buffer.
    float *zbuf = new float[options.width * options.height];
    for (int i = 0; i < options.width * options.height; i++)
      zbuf[i] = -std::numeric_limits<float>::max();

    Vec3f light_dir(0, 0, -1); // define light_dir
    for (int i = 0; i < model->nfaces(); i++) {
      // load data from model
      std::vector<int> face = model->face(i);
      Vec3f screen_coords[3]; // coord of 3 verts trace on screen plate
      Vec3f world_coords[3];  // coord of 3 verts without any transform
      Vec2f tex_coords[3];    // coord of 3 verts for texturing
      for (int j = 0; j < 3; j++) {
        Vec3f v = model->vert(face[j]);
        Vec2f tv = model->texvert(face[j]);
        world_coords[j] = v;
        screen_coords[j] = world2screen(v, options.width, options.height);
        tex_coords[j] = tv;
      }

      // calculate light intensity
      Vec3f n = (world_coords[2] - world_coords[0]) ^
                (world_coords[1] - world_coords[0]);
      n.normalize();
      float intensity = n * light_dir;

      // render on image, texturing will be done in draw_triangle()
      if (intensity > 0) {
        draw_triangle(screen_coords, tex_coords, zbuf, image, texture,
                      intensity);
      }
    }
  }

  image.flip_vertically();
  image.write_tga_file(options.outputpath.c_str());
  delete model;

  return 0;
}