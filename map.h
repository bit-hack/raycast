#pragma once
#include <cstdint>
#include <array>
#include <cstring>

#include "vector.h"


struct map_t {
  static const size_t mapWidth  = 24;
  static const size_t mapHeight = 24;

  std::array<uint8_t, mapWidth * mapHeight> floor;
  std::array<uint8_t, mapWidth * mapHeight> ceil;

  enum {
    block_left  = 1,
    block_right = 2,
    block_up    = 4,
    block_down  = 8,
  };

  void load(const uint8_t *f, const uint8_t *c) {
    memcpy(floor.data(), f, floor.size());
    memcpy(ceil.data(), c, ceil.size());
    calcBlockers();
  }

  const uint8_t getHeight(int32_t x, int32_t y) const {
    return floor[x + y * mapWidth];
  }

  const uint8_t getCeil(int32_t x, int32_t y) const {
    return ceil[x + y * mapWidth];
  }

  void resolve(const vec3f_t &p, const float r, vec2f_t &res) const;

protected:

  // blocker flags
  std::array<uint8_t, mapWidth * mapHeight> blockers;

  uint8_t getHeight_(int32_t x, int32_t y) const {
    return (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
      ? 0xff : getHeight(x, y);
  }

  void calcBlockers(void);
};
