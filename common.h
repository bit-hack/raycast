#pragma once
#include <array>

#define _SDL_main_h
#include "SDL.h"

#include "texture.h"
#include "vector.h"
#include "map.h"


extern std::array<texture_t, 16> texture;
extern map_t map;

extern vec3f_t player_pos;
extern float player_dir;
extern float eye_level;
extern const float near_plane_scale;

// screen width and height
enum { w = 320, h = 240 };

extern std::array<uint32_t, w*h> screen;
extern std::array<uint16_t, w*h> depth;


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
    return (h / 2.f) - 64 * (y - eye_level) / dist;
  }
}

inline vec2f_t project(vec3f_t &p) {

  const vec2f_t cam_dir = { sinf(player_dir), cosf(player_dir) };
  const vec2f_t plane{ cam_dir.y,-cam_dir.x };

  vec2f_t diff{p.x - player_pos.x, p.y - player_pos.y};

  const float dist = vec2f_t::dot(diff, cam_dir);

  const vec2f_t projected = diff / dist;
  const float side = vec2f_t::dot(projected, plane);

  const float x = w / 2 + w * side * near_plane_scale;
  const float y = (h / 2.f) - 64 * (p.z - eye_level) / dist;

  return vec2f_t{x, y};
}

inline float cam_distance(const vec3f_t &p) {
  // XXX: should -1 for the near plane maybe?
  const vec2f_t dir = { sinf(player_dir), cosf(player_dir) };
  const float dist = vec2f_t::dot(vec2f_t{p.x, p.y}, dir);
  return dist;
}

} // namespace {}