#include <cmath>
#include <algorithm>

#include "map.h"

#include "SDL.h"

map_t map;

void map_t::calcBlockers(void) {
  for (size_t y = 0; y < map_h; ++y) {
    for (size_t x = 0; x < map_w; ++x) {

      // XXX: check ceiling height

      // if anything two tiles below us block in that direction
      const uint8_t h = floor[x + y * map_w];
      uint8_t flags = 0;
      flags |= (getHeight_(x, y - 1) + 2) <= h ? block_up    : 0;
      flags |= (getHeight_(x, y + 1) + 2) <= h ? block_down  : 0;
      flags |= (getHeight_(x - 1, y) + 2) <= h ? block_left  : 0;
      flags |= (getHeight_(x + 1, y) + 2) <= h ? block_right : 0;
      blockers[x + y * map_w] = flags;
    }
  }
}

void map_t::resolve(const vec3f_t &p, const float r, vec2f_t &res) const {

  // player height
  const uint8_t ph = uint8_t(p.z + 0.5f);

  res = vec2f_t{ 99.f, 99.f };

  const vec2i_t min{std::max<int32_t>(int32_t(p.x - r), 0),
                    std::max<int32_t>(int32_t(p.y - r), 0)};
  const vec2i_t max{std::min<int32_t>(int32_t(p.x + r), map_w -1),
                    std::min<int32_t>(int32_t(p.y + r), map_h-1)};

  bool setx = false, sety = false;

  for (int32_t y = min.y; y <= max.y; ++y) {
    for (int32_t x = min.x; x <= max.x; ++x) {

      // test tile height
      const uint8_t h = getHeight(x, y);
      if (h <= ph + 1)
        continue;

      const uint8_t b = blockers[x + y * map_w];

      const float dy0 = (b & block_up)    ? ((y + 0.f) - (p.y + r)) : 99.f;
      const float dy1 = (b & block_down)  ? ((y + 1.f) - (p.y - r)) : 99.f;

      const float dx0 = (b & block_left)  ? ((x + 0.f) - (p.x + r)) : 99.f;
      const float dx1 = (b & block_right) ? ((x + 1.f) - (p.x - r)) : 99.f;

      const float rx = fabsf(dx0) < fabsf(dx1) ? dx0 : dx1;
      const float ry = fabsf(dy0) < fabsf(dy1) ? dy0 : dy1;

      if (fabsf(rx) < fabsf(ry)) {
        res.x = fabsf(rx) < fabsf(res.x) ? rx : res.x;
        setx = res.x < 98.f;
      }
      else {
        res.y = fabsf(ry) < fabsf(res.y) ? ry : res.y;
        sety = res.y < 98.f;
      }
    }
  }

  res.x = setx ? res.x : 0.f;
  res.y = sety ? res.y : 0.f;
}

void map_t::load(const char *fl, const char *cl, const char *lt) {

  tex_wall.fill(0);
  tex_floor.fill(1);
  tex_ceil.fill(11);

  this->floor.fill(0);
  this->floor.fill(10);
  this->light.fill(0xff);

  load_(fl, this->floor, 4);
  load_(cl, this->ceil, 4);
  load_(lt, this->light, 1);

  calcBlockers();
}

bool map_t::load_(const char *path, std::array<uint8_t, map_w*map_h> &out, uint32_t scale) const {

  SDL_Surface *map = SDL_LoadBMP(path);
  if (!map) {
    return false;
  }

  if (map->format->BitsPerPixel != 8) {
    SDL_FreeSurface(map);
    return false;
  }

  const uint8_t *src = (const uint8_t*)map->pixels;

  uint8_t *dst = out.data();
  for (int y = 0; y < map_h; ++y) {
    for (int x = 0; x < map_w; ++x) {
      dst[x] = src[x] / scale;
    }
    src += map->pitch;
    dst += map_w;
  }

  SDL_FreeSurface(map);
  return true;
}
