#include "model.h"
#include "renderer.h"
#include "tgaimage.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

std::string obj_path = "obj/african_head.obj";
std::string diffuse_path = "texture/african_head_diffuse.tga";
std::string normal_path = "texture/african_head_nm.tga";
std::string specular_path = "texture/african_head_spec.tga";
std::string output_path = "output.tga";

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
        output_path = argv[++i];
      }
    } else if (arg[0] != '-') {
      obj_path = arg;
    }
  }
  return options;
}

int main(int argc, char **argv) {
  // initialize the renderer
  RenderOptions options = parse_args(argc, argv);
  TGAImage image(options.width, options.height, TGAImage::RGB);
  Renderer renderer(image, options);

  // load model from files
  Model *model = new Model(obj_path.c_str());
  if (model == nullptr) {
    std::cerr << "Error: Can't load model from " << obj_path << std::endl;
    return 1;
  }
  renderer.set_model(model);

  // load texture maps from files
  TGAImage diffusemap, normalmap, specularmap;
  if (diffusemap.read_tga_file(diffuse_path.c_str()))
    renderer.set_texture(diffusemap, DIFFUSE);
  if (normalmap.read_tga_file(normal_path.c_str()))
    renderer.set_texture(normalmap, NORMAL);
  if (specularmap.read_tga_file(specular_path.c_str()))
    renderer.set_texture(specularmap, SPECULAR);

  renderer.render();

  renderer.save_image(output_path);
  renderer.set_model(nullptr);

  return 0;
}