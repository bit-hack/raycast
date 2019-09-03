#include "sprites.h"

void draw_sprite(sprite_t &s, const vec3f_t &pos, const float height) {
  vec2f_t p;
  const float dist = project(pos, p);
  if (dist <= .5f) {
    // behind camera so dont render
    return;
  }

  // sprite depth
  const uint16_t d = uint16_t(dist * 256);

  const float y1 = p.y;
  const float y2 = project(pos.z + height, dist);
  const float dy = y2 - y1;

  const float dx = (dy * s.w) / s.h;

  const float x1 = p.x + dx / 2;
  const float x2 = p.x - dx / 2;

  const vec2i_t min{std::max<int32_t>(0, x1), std::max<int32_t>(0, int32_t(y2))};
  const vec2i_t max{std::min<int32_t>(w, x2), std::min<int32_t>(h, int32_t(y1))};

  for (int32_t y = min.y; y < max.y; ++y) {
    for (int32_t x = min.x; x < max.x; ++x) {

      if (depth[x + y * w] < d) {
        continue;
      }

      screen[x + y * w] = 0xffffff;
    }
  }
}
