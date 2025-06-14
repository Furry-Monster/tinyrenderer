#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>

#pragma pack(push, 1)
struct TGA_Header {
  char idlength;
  char colormaptype;
  char datatypecode;
  short colormaporigin;
  short colormaplength;
  char colormapdepth;
  short x_origin;
  short y_origin;
  short width;
  short height;
  char bitsperpixel;
  char imagedescriptor;
};
#pragma pack(pop)

struct TGAColor {
  union {
    struct {
      unsigned char b, g, r, a;
    };
    unsigned char raw[4];
    unsigned int val;
  };
  unsigned char bytespp;

  TGAColor() : val(0), bytespp(1) {}

  TGAColor(unsigned char R, unsigned char G, unsigned char B,
           unsigned char A = 255)
      : b(B), g(G), r(R), a(A), bytespp(4) {}

  TGAColor(int v, int bpp) : val(v), bytespp(bpp) {}

  TGAColor(const TGAColor &c) : val(c.val), bytespp(c.bytespp) {}

  TGAColor(const unsigned char *p, int bpp) : val(0), bytespp(bpp) {
    for (int i = 0; i < bpp; i++) {
      raw[i] = p[i];
    }
  }

  TGAColor &operator=(const TGAColor &c) {
    if (this != &c) {
      bytespp = c.bytespp;
      val = c.val;
    }
    return *this;
  }

  unsigned char &operator[](const int i) { return raw[i]; }

  TGAColor operator*(float intensity) const {
    TGAColor res = *this;
    intensity =
        (intensity > 1.0f ? 1.0f : (intensity < 0.0f ? 0.0f : intensity));
    for (int i = 0; i < 4; i++) {
      res.raw[i] *= intensity;
    }
    return res;
  }
};

const TGAColor white(255, 255, 255, 255);
const TGAColor red(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const TGAColor yellow = TGAColor(255, 255, 0, 255);

class TGAImage {
protected:
  unsigned char *data;
  int width;
  int height;
  int bytespp;

  bool load_rle_data(std::ifstream &in);
  bool unload_rle_data(std::ofstream &out) const;

public:
  enum Format {
    GRAYSCALE = 1,
    RGB = 3,
    RGBA = 4,
  };

  TGAImage();
  TGAImage(int w, int h, int bpp);
  TGAImage(const TGAImage &img);
  bool read_tga_file(const char *filename);
  bool write_tga_file(const char *filename, bool rle = true) const;
  bool flip_horizontally();
  bool flip_vertically();
  bool scale(int w, int h);
  TGAColor get_pixel(int x, int y) const;
  bool set_pixel(int x, int y, TGAColor c);
  ~TGAImage();
  TGAImage &operator=(const TGAImage &img);
  int get_width() const;
  int get_height() const;
  int get_bytespp() const;
  unsigned char *buffer();
  void clear();
};

#endif //__IMAGE_H__
