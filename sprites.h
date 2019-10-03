#pragma once
#include <memory>

#include "common.h"

enum {
  SPRITE_IMP,
  SPRITE_SKY,
  SPRITE_PISTOL,
  SPRITE_GORE,
  SPRITE_DECALS,
  SPRITE_DEBUG,
};


struct sprite_t {

  bool load(const char *path);

  uint32_t w, h;
  uint32_t frame_h;
  std::unique_ptr<uint32_t[]> data;
};

extern std::array<sprite_t, 16> sprites;

void draw_sprite(
  sprite_t &s,          // sprite
  const vec3f_t &p,     // sprite location
  const float height,   // sprite height
  const uint8_t light,  // brightness
  const int32_t frame); // animation frame

void draw_sprite(
  sprite_t &s,          // sprite
  const vec2f_t &p,     // sprite location
  const uint8_t light,  // brightness
  const int32_t frame); // animation frame

bool load_sprites();
