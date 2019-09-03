#pragma once
#include <memory>

#include "common.h"


struct sprite_t {
  uint32_t w, h;
  std::unique_ptr<uint32_t> data;
};

void draw_sprite(
  sprite_t &s,          // sprite
  const vec3f_t &p,     // sprite location
  const float height);  // sprite height
