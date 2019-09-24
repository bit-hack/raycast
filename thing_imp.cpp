#include "map.h"
#include "things.h"
#include "pfields.h"

static float randf() {
  return 1.f - 2.f * (float(rand()) / float(RAND_MAX));
}

static float move_to(float dx, float speed, float thresh) {
  return (dx > thresh) ? speed : ((dx < thresh) ? -speed : 0.f);;
}

void thing_imp_t::on_create() {
  acc = vec3f_t{0.f, 0.f, 0.f};
  dir = 1.f;
}

void thing_imp_t::replan() {

  const float px = pos.x + randf() * 2.f;
  const float py = pos.y + randf() * 2.f;

  const uint8_t n = service.pfield->get(int(px), int(py));

  if (n >= service.pfield->get(int(target.x), int(target.y))) {
    target.x = px;
    target.y = py;
  }
}

void thing_imp_t::tick() {

  {
    replan();
    const float dx = target.x - pos.x;
    const float dy = target.y - pos.y;
    const float d = dx*dx + dy*dy;
    if (d > .1f) {
      acc.x += move_to(dx, 0.003f, 0.01f);
      acc.y += move_to(dy, 0.003f, 0.01f);
    }
  }

  const uint8_t h = service.map->getHeight(int(pos.x), int(pos.y));
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

  const uint8_t l = service.map->getLight(int(pos.x), int(pos.y));

  draw_sprite(sprites[0], pos, 4, l, 0);

  vec2f_t p;
  if (project(vec3f_t{target.x, target.y, pos.z}, p) > 0) {
    plot(int(p.x), int(p.y), 0xFFFFFF);
  }
}

thing_t *thing_create_imp() {
  return new thing_imp_t;
}
