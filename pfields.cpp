#include "pfields.h"

bool pfield_t::is_passable(int x, int y, uint8_t h) {
  const uint8_t t = map.getHeight(x, y);
  return (t > h) ? ((t - h) <= 1) : ((h - t) <= 1);
}

void pfield_t::update() {

  const uint8_t *src = cell[ff].data();
  ff ^= 1;
  uint8_t *dst = cell[ff].data();

  for (int y=1; y<map_h-1; ++y) {

    src += map_w;
    dst += map_w;

    for (int x=1; x<map_w-1; ++x) {

      const uint8_t h = map.getHeight(x, y);

      const uint8_t cc = src[x];
      const uint8_t y0 = is_passable(x, y-1, h) ? src[x + (y - map_w)] : 0;
      const uint8_t y1 = is_passable(x, y+1, h) ? src[x + (y + map_w)] : 0;
      const uint8_t x0 = is_passable(x-1, y, h) ? src[x - 1] : 0;
      const uint8_t x1 = is_passable(x+1, y, h) ? src[x + 1] : 0;

      dst[x] = std::max({x0, x1, y0, y1, cc}) - 1;
    }
  }
}
