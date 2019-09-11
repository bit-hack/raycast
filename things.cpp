#include "map.h"
#include "things.h"
#include "pfields.h"

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

static float randf() {
  return 1.f - 2.f * (float(rand()) / float(RAND_MAX));
}

void thing_imp_t::replan() {

  uint8_t best = 0;

  for (int i = 0; i < 8; ++i) {
    const float px = pos.x + randf() * 2.f;
    const float py = pos.y + randf() * 2.f;

    const uint8_t n = service.pfield->get(px, py);

    if (n >= best) {
      best = n;
      target.x = px;
      target.y = py;
    }
  }
}

void thing_imp_t::tick() {

  {
    const float dx = target.x - pos.x;
    const float dy = target.y - pos.y;
    const float d = dx*dx + dy*dy;
    if (d > 2.f || d < .5f) {
      replan();
    } else {
      acc.x += dx * 0.003f;
      acc.y += dy * 0.003f;
    }
  }

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

  const uint8_t l = service.map->getLight(pos.x, pos.y);

  draw_sprite(sprites[0], pos, 4, l);
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
