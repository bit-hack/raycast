#pragma once
#include <cmath>
#include <array>

#define _SDL_main_h
#include "SDL.h"

#include "texture.h"
#include "vector.h"


extern std::array<texture_t, 16> texture;
extern struct map_t map;

#if 0
extern vec3f_t player_pos;
extern float player_dir;
extern float eye_level;
#endif

extern const float near_plane_scale;

const uint32_t ticks_per_sec = 30;

// screen width and height
enum {
  screen_w = 320,
  screen_h = 240
};

extern std::array<uint32_t, screen_w*screen_h> screen;
extern std::array<uint16_t, screen_w*screen_h> depth;
extern std::array<uint8_t, screen_w*screen_h> lightmap;

float player_eye_level();
vec3f_t &player_pos();
vec2f_t player_dir();
float player_angle();

namespace {

inline float fpart(float x) {
  return x - float(int32_t(x));
}

inline float ipart(float x) {
  return float(int32_t(x));
}

inline float project(float y, float dist, float def = screen_h) {
  if (dist < 0.01f) {
    return def;
  }
  else {
    return (screen_h / 2.f) - 64 * (y - player_eye_level()) / dist;
  }
}

inline float project(const vec3f_t &p, vec2f_t &out) {

  const vec2f_t cam_dir = player_dir();
  const vec2f_t plane{ cam_dir.y,-cam_dir.x };

  const vec3f_t &pos = player_pos();

  vec2f_t diff{p.x - pos.x, p.y - pos.y};

  const float dist = vec2f_t::dot(diff, cam_dir);

  const vec2f_t projected = diff / dist;
  const float side = vec2f_t::dot(projected, plane);

  // XXX: no idea why I need the fudge yet
  const float fudge = 1.1f;

  const float x = screen_w / 2 + screen_w * side * near_plane_scale * fudge;
  const float y = (screen_h / 2.f) - 64 * (p.z - player_eye_level()) / dist;

  out = vec2f_t{x, y};
  return dist;
}

inline float cam_distance(const vec3f_t &p) {
  // XXX: should -1 for the near plane maybe?
  const vec2f_t dir = player_dir();
  const float dist = vec2f_t::dot(vec2f_t{p.x, p.y}, dir);
  return dist;
}

} // namespace {}

void present_screen(SDL_Surface *surf);
void present_screen_sse(SDL_Surface *surf);
void present_depth(SDL_Surface *surf);

struct service_t {
  struct map_t *map;
  struct thing_manager_t *things;
  struct pfield_t *pfield;
  struct spatial_t *spatial;
};

extern service_t service;

void plot(int x, int y, uint32_t rgb);
