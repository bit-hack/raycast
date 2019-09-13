#pragma once
#include <cstdint>
#include <array>
#include <cstring>
#include <string>

#include "vector.h"

enum {
  map_w = 128,
  map_h = 128
};


struct map_t {
  std::array<uint8_t, map_w * map_h> floor;
  std::array<uint8_t, map_w * map_h> ceil;
  std::array<uint8_t, map_w * map_h> light;

  std::array<uint8_t, map_w * map_h> tex_wall;
  std::array<uint8_t, map_w * map_h> tex_floor;
  std::array<uint8_t, map_w * map_h> tex_ceil;

  enum {
    block_left  = 1,
    block_right = 2,
    block_up    = 4,
    block_down  = 8,
  };

  void load(const std::string &path);

  const uint8_t getHeight(int32_t x, int32_t y) const {
    return floor[x + y * map_w];
  }

  const uint8_t getCeil(int32_t x, int32_t y) const {
    return ceil[x + y * map_w];
  }

  const uint8_t getLight(int32_t x, int32_t y) const {
    return light[x + y * map_w];
  }

  void resolve(const vec3f_t &p, const float r, vec2f_t &res) const;

protected:

  bool load_(const std::string &path, std::array<uint8_t, map_w*map_h> &out, uint32_t scale) const;

  // blocker flags
  std::array<uint8_t, map_w * map_h> blockers;

  uint8_t getHeight_(int32_t x, int32_t y) const {
    return (x < 0 || x >= map_w || y < 0 || y >= map_h)
      ? 0xff : getHeight(x, y);
  }

  void calcBlockers(void);
};
