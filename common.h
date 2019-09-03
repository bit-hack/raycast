#pragma once
#include <array>

#include "texture.h"
#include "vector.h"
#include "map.h"


extern std::array<texture_t, 16> texture;
extern map_t map;

extern vec3f_t player_pos;
extern float eyeLevel;

enum { w = 320, h = 240 };

namespace {

inline float fpart(float x) {
  return x - float(int32_t(x));
}

inline float ipart(float x) {
  return float(int32_t(x));
}

inline float project(float y, float dist) {
  if (dist < 0.01f) {
    return float(h);
  }
  else {
    return (h / 2.f) - 64 * (y - eyeLevel) / dist;
  }
}

} // namespace {}
