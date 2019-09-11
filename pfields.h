#pragma once
#include <array>
#include <cstdint>

#include "map.h"

struct pfield_t {

  pfield_t(map_t &m)
    : map(m)
    , ff(0)
  {
    cell[0].fill(0);
    cell[1].fill(0);
  }

  void update();

  void set(int x, int y, uint8_t val) {
    if ((x >= 1) && x < (map_w - 1) && (y >= 1) && (map_h - 1)) {
      (cell[ff])[ x + y * map_w ] = val;
    }
  }

  uint8_t get(int x, int y) const {
    if ((x >= 1) && x < (map_w - 1) && (y >= 1) && (map_h - 1)) {
      return (cell[ff])[ x + y * map_w ];
    }
    return 0;
  }

protected:

  const map_t &map;

  bool is_passable(int x, int y, uint8_t h);

  std::array<uint8_t, map_w * map_h> cell[2];
  int ff;
};
