#pragma once
#include <memory>

#include "common.h"


struct sprite_t {

  bool load(const char *path);

  uint32_t w, h;
  std::unique_ptr<uint32_t[]> data;
};

extern std::array<sprite_t, 16> sprites;

void draw_sprite(
  sprite_t &s,          // sprite
  const vec3f_t &p,     // sprite location
  const float height,   // sprite height
  const uint8_t light); // brightness

bool load_sprites();
