#include "sprites.h"

std::array<sprite_t, 16> sprites;

bool sprite_t::load(const char *path) {

  const uint32_t key = 0xff00ff;

  SDL_Surface *bmp = SDL_LoadBMP(path);
  if (!bmp) {
    return false;
  }

  if (bmp->format->BitsPerPixel != 24) {
    SDL_FreeSurface(bmp);
    return false;
  }

  w = bmp->w;
  h = bmp->h;
  data.reset(new uint32_t[w * h]);

  const uint8_t *src = (const uint8_t *)bmp->pixels;
  uint32_t *dst = data.get();

  for (uint32_t y = 0; y < h; ++y) {
    for (uint32_t x = 0; x < w; ++x) {
      const uint32_t r = src[x * 3 + 2];
      const uint32_t g = src[x * 3 + 1];
      const uint32_t b = src[x * 3 + 0];

      const uint32_t rgb = (r << 16) | (g << 8) | b;

      dst[x] = (rgb == key ? 0xff000000 : rgb);
    }
    src += bmp->pitch;
    dst += w;
  }

  SDL_FreeSurface(bmp);

  return true;
}

void draw_sprite(sprite_t &s, const vec3f_t &pos, const float height, const uint8_t light) {
  vec2f_t p;
  const float dist = project(pos, p);
  if (dist <= .5f) {
    // behind camera so dont render
    return;
  }

  // sprite depth
  const uint16_t d = uint16_t(dist * 256);

  const float y2 = p.y;
  const float y1 = project(pos.z + height, dist);
  const float dy = y1 - y2;

  const float dx = (dy * s.w) / s.h;

  const vec2f_t box_min{p.x + dx / 2, y1};
  const vec2f_t box_max{p.x - dx / 2, y2};

  const vec2i_t min{std::max<int32_t>(0,        int32_t(box_min.x)),
                    std::max<int32_t>(0,        int32_t(box_min.y))};
  const vec2i_t max{std::min<int32_t>(screen_w, int32_t(box_max.x)),
                    std::min<int32_t>(screen_h, int32_t(box_max.y))};

  const float sx = float(s.w) / (box_max.x - box_min.x);
  const float sy = float(s.h) / (box_max.y - box_min.y);

  float tx = (box_min.x < 0) ? sx * -box_min.x : 0.f;
  float ty = (box_min.y < 0) ? sy * -box_min.y : 0.f;

  const uint32_t *src = s.data.get();

  uint8_t *lit = lightmap.data();

  if (min.x > max.x) return;
  if (min.y > max.y) return;
  if (max.x < min.x) return;
  if (max.y < min.y) return;

  for (int32_t y = min.y; y < max.y; ++y, ty += sy) {
    tx = (box_min.x < 0) ? sx * -box_min.x : 0.f;
    for (int32_t x = min.x; x < max.x; ++x, tx += sx) {
      // depth test
      if (depth[x + y * screen_w] < d) {
        continue;
      }
      // look up texel
      const uint32_t rgb = src[ int32_t(tx) + int32_t(ty) * s.w ];
      // alpha test
      if ((rgb & 0xff000000) == 0) {
        screen[x + y * screen_w] = rgb;
        depth [x + y * screen_w] = d;
        lit   [x + y * screen_w] = light;
      }
    }
  }
}

bool load_sprites() {
  FILE *f = fopen("data/sprites.txt", "r");
  if (!f) {
    return true;
  }
  std::array<char, 256> temp;
  for (int i = 0; i < sprites.size() && !feof(f); ++i) {
    temp[0] = '\0';
    if (!fgets(temp.data(), temp.size(), f)) {
      break;
    }
    temp[255] = '\0';
    temp[strcspn(temp.data(), "\n" )] = '\0';
    if (!sprites[i].load(temp.data())) {
      // oh dear
    }
  }
  return true;
}
