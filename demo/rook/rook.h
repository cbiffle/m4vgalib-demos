#ifndef DEMO_ROOK_ROOK_H
#define DEMO_ROOK_ROOK_H

#include "etl/math/matrix.h"
#include "etl/math/vector.h"

#include "vga/vga.h"
#include "vga/rast/bitmap_1.h"
#include "vga/rast/solid_color.h"
#include "vga/rast/text_10x16.h"
#include "vga/font_10x16.h"

#include "demo/scene.h"
#include "demo/rook/config.h"

namespace demo {
namespace rook {

struct Wireframe {
  vga::rast::Bitmap_1 rasterizer{config::cols,
                                 config::wireframe_rows,
                                 config::top_margin};
  etl::math::Vec2i * transformed_vertices;

  Wireframe();
  ~Wireframe();

  void transform_vertices(etl::math::Mat4f const &m) const;
  void draw_edges(vga::Graphics1 &g);
};

struct BragLine {
  unsigned message[81];
  unsigned t_c = 0;
  vga::rast::Text_10x16 text{
    vga::font_10x16, 256,
    config::cols + 10,
    config::text_rows,
    config::rows - config::text_rows,
    true};

  BragLine();
  void show_msg(unsigned frame);

private:
  void string(vga::Rasterizer::Pixel fore,
              vga::Rasterizer::Pixel back,
              char const *s);
  
};

class Rook : public Scene {
public:
  Rook();

  void configure_band_list() override;
  bool render_frame(unsigned) override;

private:
  vga::rast::SolidColor _blue{config::cols, 0b010000};
  Wireframe _wireframe;
  BragLine _brag_line;

  vga::Band const _bands[4] {
    { &_blue,                 config::top_margin,        &_bands[1] },
    { &_wireframe.rasterizer, config::wireframe_rows,    &_bands[2] },
    { &_blue,                 config::bottom_margin,     &_bands[3] },
    { &_brag_line.text,       config::text_rows,         nullptr },
  };

  etl::math::Mat4f _projection;
  etl::math::Mat4f _model;
};

void legacy_run();

}  // namespace rook
}  // namespace demo

#endif  // DEMO_ROOK_ROOK_H
