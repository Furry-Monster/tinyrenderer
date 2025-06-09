#include "gmath.h"
#include "model.h"
#include "renderer.h"
#include "tgaimage.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

void print_usage() {
  std::cout
      << "Usage: tinyrenderer [Options] <filepath>\n"
      << "Options:\n"
      << "  -m, --mode     RenderMode (line/triangle/zbuf/textured/shading, "
         "默认: line)\n"
      << "  -w, --width    Width for output image (默认: 800)\n"
      << "  -h, --height   Height for output image (默认: 800)\n"
      << "  -d, --depth    Max depth for rendering (默认: 255)\n"
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
        } else if (mode == "zbuf") {
          options.mode = RenderingMode::ZBUF;
        } else if (mode == "textured") {
          options.mode = RenderingMode::TEXTURED;
        } else if (mode == "shading") {
          options.mode = RenderingMode::SHADING;
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
    } else if (arg == "-d" || arg == "--depth") {
      if (i + 1 < argc) {
        options.depth = std::stoi(argv[++i]);
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
 * @brief Triangle drawing function from trianglebench_main.cpp:draw_triangle4()
 *
 * @param pts point set
 * @param zbuf zbuffer reference for depth testing
 * @param image image to draw
 * @param color color for image (without interpolate)
 */
void draw_triangle(Vec3f *pts, float *zbuf, TGAImage &image,
                   TGAColor color) noexcept {
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
                   TGAImage &image, Model &model, float intensity) noexcept {
  int xmax = -1, ymax = -1;
  int xmin = 8000, ymin = 8000; // i dont think somebody would use 8k screen...
  Vec2i screen_2i[3];

  for (int i = 0; i < 3; i++) {
    Vec2i cur_sc = Vec2i(screen_coords[i]);
    screen_2i[i] = cur_sc;

    if (cur_sc.x < xmin)
      xmin = cur_sc.x;
    if (cur_sc.x > xmax)
      xmax = cur_sc.x;
    if (cur_sc.y < ymin)
      ymin = cur_sc.y;
    if (cur_sc.y > ymax)
      ymax = cur_sc.y;
  }

  Vec3f pixelPos, bc_screen;
  for (int i = xmin; i < xmax; i++) {
    for (int j = ymin; j < ymax; j++) {
      pixelPos.x = i;
      pixelPos.y = j;
      pixelPos.z = 0.0f;

      bc_screen = barycentric2d(screen_2i, pixelPos.toVec2());
      // Do barycentric test then.
      // If barycentric have any negative value,
      // the pixelPos would be regard to be outlined,
      // although it's inside bounding box
      if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
        continue;

      // barycentric interpolate texturing
      Vec2f tex_pos(0, 0);
      for (int k = 0; k < 3; k++) {
        tex_pos.x += tex_coords[k].x * bc_screen[k];
        tex_pos.y += tex_coords[k].y * bc_screen[k];
      }
      TGAColor color = model.uv(tex_pos, Model::DIFFUSE);

      // an easy lighting,well, it's not enough for me...
      color = TGAColor(color.r * intensity, color.g * intensity,
                       color.b * intensity, color.a);

      // depth buffer testing here.
      for (int k = 0; k < 3; k++)
        pixelPos.z += screen_coords[k].z * bc_screen[k];
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

int main(int argc, char **argv) {
  // initialize the renderer
  RenderOptions options = parse_args(argc, argv);
  TGAImage image(options.width, options.height, TGAImage::RGB);
  Renderer renderer(image, options);

  // load model and textures from files
  Model *model = new Model(options.objpath.c_str());
  if (model == nullptr) {
    std::cerr << "Error: Can't load model from " << options.objpath
              << std::endl;
    return 1;
  }
  model->load_texture(options.diffusepath, Model::DIFFUSE);
  model->load_texture(options.normalpath, Model::NORMAL);
  model->load_texture(options.specularpath, Model::SPECULAR);

  renderer.set_model(model);
  renderer.render();

  renderer.save_output();
  renderer.set_model(nullptr);

  return 0;
}