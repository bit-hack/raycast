#pragma once
#include <array>
#include <vector>

#include "vector.h"
#include "sprites.h"
#include "common.h"

const float gravity = 0.1f;
const float damping = 0.9f;

enum thing_type_t {
  IMP,
  FIREBALL,

  PLAYER,
  _NUM_THINGS_
};

struct thing_t {

  thing_t(thing_type_t t)
    : type(t)
    , pos(vec3f_t{0.f, 0.f, 0.f})
    , acc(vec3f_t{0.f, 0.f, 0.f})
    , dir(0.f)
    , radius(0.3f)
    , height(3.f)
  {
  }

  virtual void tick() {};
  virtual void on_create() {};
  virtual void on_damage(
    const vec3f_t &src,
    const vec3f_t &pos,
    float damage) {};

  void move(const vec3f_t &p);

  const thing_type_t type;

  virtual bool screen_aabb(vec2f_t &min, vec2f_t &max) const {
    return false;
  }

  vec3f_t pos;
  vec3f_t acc;
  float dir;
  float radius;
  float height;
};

struct thing_manager_t {

  void tick();

  thing_t *create(thing_type_t type);

  std::vector<thing_t *> &get_things(thing_type_t type) {
    return things[type];
  }

protected:
  std::array<std::vector<thing_t*>, _NUM_THINGS_> things;
};

struct thing_player_t: public thing_t {

  thing_player_t()
    : thing_t(PLAYER)
  {}

  void on_create() override;
  void tick() override;

  float eyeLevel;
  float viewBob;
  float gunBob;
  float accMag;
  float gunFrame;

protected:
  void draw_gun();
  void do_movement();
  void do_shoot();
};

struct thing_imp_t: public thing_t {

  vec2f_t target;

  thing_imp_t()
    : thing_t(IMP)
    , target(vec2f_t{999.f, 999.f})
  {}

  bool screen_aabb(vec2f_t &min, vec2f_t &max) const override;

  void on_create() override;
  void tick() override;

  virtual void on_damage(
    const vec3f_t &src,
    const vec3f_t &hit,
    float damage);

protected:
  void draw();
  void replan();
};
