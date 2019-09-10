#include "map.h"
#include "things.h"

const float gravity = 0.1f;
const float damping = 0.9f;

thing_t::thing_t(thing_type_t t)
  : type(t)
{
}

void thing_manager_t::tick() {
  for (auto &t : things) {
    t->tick();
  }
}

void thing_imp_t::on_create() {
  acc = vec3f_t{0.f, 0.f, 0.f};
  dir = vec2f_t{1.f, 0.f};
}

void thing_imp_t::tick() {

  const uint8_t h = service.map->getHeight(pos.x, pos.y);
  if (pos.z <= h) {
    pos.z = h;
    acc.z = 0.f;
  }
  else {
    acc.z -= gravity;
  }

  pos += acc;
  acc *= 0.9f;

  vec2f_t res = {0.f, 0.f};
  map.resolve(pos, 0.3f, res);
  pos.x += res.x * 0.5f;
  pos.y += res.y * 0.5f;

  // TODO: sample brightness

  draw_sprite(sprites[0], pos, 4, 0xff);
}

thing_t *thing_manager_t::create(thing_type_t type) {
  thing_t *t = nullptr;
  switch (type) {
  case IMP:     t = new thing_imp_t; break;
  case PLAYER:  t = new thing_player_t; break;
  }
  if (t) {
    t->on_create();
    things.push_back(t);
  }
  return t;
}
