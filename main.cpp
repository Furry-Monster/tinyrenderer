#include "gmath.h"
#include "gutils.h"
#include "model.h"
#include "tgaimage.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

enum RenderingMode {
  LINE,
  TRIANGLE,
  ZBUF,
  TEXTURING,
  SHADING,
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
  int depth = 255;
};

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
          options.mode = RenderingMode::TEXTURING;
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
 * @brief Get a matrix for transformation from NDC to Screen/Viewport
 *
 * @param x Viewport origin x value
 * @param y Viewport origin y value
 * @param w Viewport width
 * @param h Viewport height
 * @param depth Depth for Culling and Depth-Testing
 * @return Mat4f Viewport Transformation Matrix
 */
Mat4f viewport_trans(int x, int y, int w, int h, int depth) noexcept {
  Mat4f m = {
      {w / 2.0f, 0, 0, x + w / 2.0f},
      {0, h / 2.0f, 0, y + h / 2.0f},
      {0, 0, depth / 2.0f, depth / 2.0f},
      {0, 0, 0, 1},
  };
  return m;
}

/**
 * @brief Find 2d coord's barycentric.
 *
 * @param pts triangle set
 * @param P P for finding barycentric
 * @return Vec3f uv_map , structure {1,y,x} , be careful it's inverse :-)
 */
Vec3f barycentric2d(Vec2i *pts, Vec2f P) noexcept {
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
                   TGAImage &image, const TGAImage &texture,
                   float intensity) noexcept {
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
      int tex_x = tex_pos.x * texture.get_width();
      int tex_y = tex_pos.y * texture.get_height();
      TGAColor color = texture.get(tex_x, tex_y);

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
  texture.flip_vertically();

  if (options.mode == RenderingMode::LINE) {
    for (int i = 0; i < model->v_ind_num(); i++) {
      std::vector<int> face = model->getv_ind(i);
      for (int j = 0; j < 3; j++) {
        Vec3f v0 = model->getv(face[j]);
        Vec3f v1 = model->getv(face[(j + 1) % 3]);
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
    Vec3f camera(0, 0, 3);     // define camera position;

    Mat4f projection = Mat4f::identity();
    projection[3][2] = -1.0f / camera.z;
    Mat4f viewport = viewport_trans(
        options.width * 1 / 8, options.height * 1 / 8, options.width * 3 / 4,
        options.height * 3 / 4, options.depth);

    // render each piece/triangles
    for (int i = 0; i < model->v_ind_num(); i++) {
      // load data from model
      std::vector<int> face = model->getv_ind(i);
      Vec3f screen_coords[3]; // coord of 3 verts trace on screen plate
      Vec3f world_coords[3];  // coord of 3 verts without any transform
      for (int j = 0; j < 3; j++) {
        Vec3f v = model->getv(face[j]);
        world_coords[j] = v;
        screen_coords[j] = m2v(viewport * projection * v2m(v));
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
    delete[] zbuf;
  } else if (options.mode == RenderingMode::ZBUF) {
    // allocate depth buffer.
    float *zbuf = new float[options.width * options.height];
    for (int i = 0; i < options.width * options.height; i++)
      zbuf[i] = -std::numeric_limits<float>::max();

    Vec3f camera(0, 0, 3); // define camera position;
    TGAImage zbufimage(options.width, options.height, TGAImage::GRAYSCALE);

    Mat4f projection = Mat4f::identity();
    projection[3][2] = -1.0f / camera.z;
    Mat4f viewport = viewport_trans(
        options.width * 1 / 8, options.height * 1 / 8, options.width * 3 / 4,
        options.height * 3 / 4, options.depth);

    // render each piece/triangles
    for (int i = 0; i < model->v_ind_num(); i++) {
      // load data from model
      std::vector<int> face = model->getv_ind(i);
      Vec3f screen_coords[3]; // coord of 3 verts trace on screen plate
      for (int j = 0; j < 3; j++) {
        Vec3f v = model->getv(face[j]);
        screen_coords[j] = m2v(viewport * projection * v2m(v));
      }

      // render on image, triangle as piece
      draw_triangle(screen_coords, zbuf, image, white);
    }

    // render finally z buffer preview image
    for (int i = 0; i < options.width; i++) {
      for (int j = 0; j < options.height; j++) {
        zbufimage.set(i, j, TGAColor(zbuf[i + j * options.width], 1));
      }
    }

    // we don't need the triangle image, so let's quit here.
    zbufimage.flip_vertically();
    zbufimage.write_tga_file(options.outputpath.c_str());
    delete model;
    delete[] zbuf;
    return 0;

  } else if (options.mode == RenderingMode::TEXTURING) {
    // allocate depth buffer.
    float *zbuf = new float[options.width * options.height];
    for (int i = 0; i < options.width * options.height; i++)
      zbuf[i] = -std::numeric_limits<float>::max();

    Vec3f light_dir(0, 0, -1); // define light_dir
    Vec3f camera(0, 0, 3);     // define camera position;

    Mat4f projection = Mat4f::identity();
    projection[3][2] = -1.0f / camera.z;
    Mat4f viewport = viewport_trans(
        options.width * 1 / 8, options.height * 1 / 8, options.width * 3 / 4,
        options.height * 3 / 4, options.depth);

    // render each face/piece
    for (int i = 0; i < model->face_num(); i++) {
      // load data from model
      std::vector<int> v_ind_tuple = model->getv_ind(i);
      std::vector<int> vt_ind_tuple = model->getvt_ind(i);
      std::vector<int> vn_ind_tuple = model->getvn_ind(i);

      Vec3f screen_coords[3]; // coord of 3 verts trace on viewport plateform
      Vec3f world_coords[3];  // coord of 3 verts without any transform
      Vec2f tex_coords[3];    // coord of 3 verts for texturing
      Vec3f norm_coords[3];   // coord of 3 vertex for lighting

      for (int j = 0; j < 3; j++) {
        Vec3f v = model->getv(v_ind_tuple[j]);
        Vec2f vt = model->getvt(vt_ind_tuple[j]);
        Vec3f vn = model->getvn(vn_ind_tuple[j]);

        world_coords[j] = v;
        screen_coords[j] = m2v(viewport * projection * v2m(v));
        tex_coords[j] = vt;
        norm_coords[j] = vn;
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
    delete[] zbuf;
  } else if (options.mode == RenderingMode::SHADING) {
    std::cout << "Not implemented yet" << std::endl;
    delete model;
    return 1;
  }

  image.flip_vertically();
  image.write_tga_file(options.outputpath.c_str());
  delete model;

  return 0;
}