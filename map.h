#pragma once
#include <cstdint>
#include <array>
#include <cstring>

#include <SDL/SDL.h>

#include "vector.h"


struct map_t {
  static const size_t mapWidth  = 24;
  static const size_t mapHeight = 24;

  std::array<uint8_t, mapWidth * mapHeight> height;
  std::array<uint8_t, mapWidth * mapHeight> blockers;

  enum {
    block_left  = 1,
    block_right = 2,
    block_up    = 4,
    block_down  = 8
  };

  void load(const uint8_t *data) {
    memcpy(height.data(), data, height.size());
    calcBlockers();
  }

  const uint8_t getHeight(int32_t x, int32_t y) const {
    return height[x + y * mapWidth];
  }

  void resolve(const vec3f_t &p, const float r, vec2f_t &res) const;

protected:
  uint8_t getHeight_(int32_t x, int32_t y) const {
    return (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
      ? 0xff : getHeight(x, y);
  }

  void calcBlockers(void);
};
