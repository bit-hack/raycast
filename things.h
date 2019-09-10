#pragma once
#include <vector>

#include "vector.h"
#include "sprites.h"
#include "common.h"

enum thing_type_t {
  PLAYER,
  IMP,
  FIREBALL,
};

struct thing_t {

  thing_t(thing_type_t t);

  virtual void tick() {};
  virtual void on_create() {};

  const thing_type_t type;

  vec3f_t pos;
  vec3f_t acc;
  vec2f_t dir;
};

struct thing_manager_t {

  void tick();

  thing_t *create(thing_type_t type);

  std::vector<thing_t*> things;
};

struct thing_player_t: public thing_t {

  thing_player_t()
    : thing_t(PLAYER)
  {}
};

struct thing_imp_t: public thing_t {

  thing_imp_t()
    : thing_t(IMP)
  {}

  void on_create() override;
  void tick() override;
};
