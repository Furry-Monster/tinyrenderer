#include "tgaimage.h"

int main(int argc, char **argv) {
  TGAImage image(800, 800, TGAImage::RGB);
  TGAColor red(255, 0, 0, 255);
  return 0;
}