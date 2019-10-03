#pragma once
#include <vector>
#include <cstdint>

#include "vector.h"

struct particle_manager_t {

  void tick();
  void spawn(const vec3f_t &pos, int8_t sprite, int32_t index, int32_t age);

protected:
  struct particle_t {
    int32_t age;
    vec3f_t acc;
    vec3f_t pos;
    int8_t sprite;
    int8_t index;
  };

  std::vector<particle_t> list;
};
