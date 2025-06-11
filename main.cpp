#include "model.h"
#include "rasterizer.h"
#include "tgaimage.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

struct FilePath {
  std::string obj = "obj/african_head.obj";
  std::string diffuse = "texture/african_head_diffuse.tga";
  std::string normal = "texture/african_head_nm.tga";
  std::string specular = "texture/african_head_spec.tga";
  std::string output = "output.tga";
} path;

/**
 * @brief print usage text on terminal
 *
 */
void print_usage() {
  std::cout
      << "Usage: tinyrenderer [Options] <filepath>\n"
      << "Options:\n"
      << "  -m, --mode     RenderMode "
         "(wireframe/zbuf/triangle/textured/shading, "
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
        if (mode == "wireframe") {
          options.mode = RenderingMode::WIREFRAME;
        } else if (mode == "zbuf") {
          options.mode = RenderingMode::ZBUFGRAY;
        } else if (mode == "triangle") {
          options.mode = RenderingMode::TRIANGLE;
        } else if (mode == "textured") {
          options.mode = RenderingMode::TRIANGLE;
          options.shadingmode = ShadingType::DIFFUSE;
        } else if (mode == "shading") {
          options.mode = RenderingMode::TRIANGLE;
          options.shadingmode = ShadingType::DIFFUSE | ShadingType::NORMAL |
                                ShadingType::SPECULAR;
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
        path.output = argv[++i];
      }
    } else if (arg[0] != '-') {
      path.obj = arg;
    }
  }
  return options;
}

int main(int argc, char **argv) {
  // initialize the renderer
  RenderOptions options = parse_args(argc, argv);
  TGAImage image(options.width, options.height, TGAImage::RGB);
  Rasterizer rst(options);

  // load model from files , owner scope of model will be set to renderer after
  // passing to constructor of renderer or calling set_model() function
  Model *model = new Model(path.obj);
  if (model == nullptr) {
    std::cerr << "Error: Can't load model from " << path.obj << std::endl;
    return 1;
  }
  rst.bind_model(model);

  // load texture maps from files
  TGAImage diffusemap, normalmap, specularmap;
  if (diffusemap.read_tga_file(path.diffuse.c_str()))
    rst.bind_texture(diffusemap, DIFFUSE);
  if (normalmap.read_tga_file(path.normal.c_str()))
    rst.bind_texture(normalmap, NORMAL);
  if (specularmap.read_tga_file(path.specular.c_str()))
    rst.bind_texture(specularmap, SPECULAR);

  // create and load shaders(here we just use "hard shader")

  // now we really need to start rendering
  rst.render();

  // save image and release model, it won't be used anymore
  rst.save_frame(path.output);
  rst.bind_model(nullptr);

  return 0;
}