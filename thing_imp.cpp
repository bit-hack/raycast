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
  height = 4.f;
  radius = 0.3f;
}

void thing_imp_t::replan() {

  const float px = pos.x + randf() * .95f;
  const float py = pos.y + randf() * .95f;

  // get imp height
  const uint8_t h0 = service.map->getHeight(int(pos.x), int(pos.y));
  const uint8_t h1 = service.map->getHeight(int(px), int(py));

  if (h0 + 1 < h1) {
    return;
  }

  const uint8_t n = service.pfield->get(int(px), int(py));

  if (n >= service.pfield->get(int(target.x), int(target.y))) {
    target.x = px;
    target.y = py;
  }
}

void thing_imp_t::on_damage(
  const vec3f_t &src,
  const vec3f_t &hit,
  float damage) {

  const vec3f_t d = normal(src - pos);
  acc = acc - d * damage;
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

  vec3f_t new_pos = pos + acc;

  const uint8_t h = service.map->getHeight(int(pos.x), int(pos.y));
  if (new_pos.z <= h) {
    new_pos.z = h;
    acc.z = 0.f;
  }
  else {
    acc.z -= gravity;
  }

  // movement damping
  acc *= 0.9f;
  // collision resolution
  vec2f_t res = {0.f, 0.f};
  map.resolve(new_pos, 0.3f, res);
  new_pos.x += res.x * 0.5f;
  new_pos.y += res.y * 0.5f;
  // move to new location
  move(new_pos);
  // draw sprite
  const uint8_t l = service.map->getLight(int(pos.x), int(pos.y));
  draw_sprite(sprites[0], pos, height, l, 0);
  // draw target
#if 0
  vec2f_t p;
  if (project(vec3f_t{target.x, target.y, pos.z}, p) > 0) {
    plot(int(p.x), int(p.y), 0xFFFFFF);
  }
#endif
#if 0
  {
    vec2f_t min, max;
    if (screen_aabb(min, max)) {
      plot(min.x, min.y, 0xFFFFFF);
      plot(min.x, max.y, 0xFFFFFF);
      plot(max.x, min.y, 0xFFFFFF);
      plot(max.x, max.y, 0xFFFFFF);
    }
  }
#endif
}

thing_t *thing_create_imp() {
  return new thing_imp_t;
}

bool thing_imp_t::screen_aabb(vec2f_t &min, vec2f_t &max) const {

  sprite_t &s = sprites[0];

  // XXX: code mirrored from sprite draw

  vec2f_t p;
  const float dist = project(pos, p);
  if (dist <= .15f) {
    // behind camera so dont render
    return false;
  }

  // sprite depth
  const uint16_t d = uint16_t(dist * 256);

  const float y2 = p.y;
  const float y1 = project(pos.z + height, dist);
  const float dy = y1 - y2;

  const float dx = (dy * s.w) / s.h;

  min = vec2f_t{p.x + dx / 2, y1};
  max = vec2f_t{p.x - dx / 2, y2};

  return (min.x < max.x) && (min.y < max.y);
}
