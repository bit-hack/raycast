#pragma once
#include <array>
#include <vector>
#include <set>

#include "map.h"
#include "things.h"
#include "common.h"

struct spatial_t {

  spatial_t(map_t &m) : map(m) {}

  void hitscan(float px, float py, float dx, float dy, vec3f_t &hit, thing_t *&t) const;
  bool LOS(const vec3f_t &a, const vec3f_t &b) const;

  void insert(thing_t *t);
  void remove(thing_t *t);

  std::vector<thing_t *> &find(uint32_t x, uint32_t y);

  void overlaps(std::set<std::pair<thing_t*, thing_t*>> &out);

  void draw();

protected:
  std::array<std::vector<thing_t *>, map_w * map_h> cells;
  map_t &map;
};
