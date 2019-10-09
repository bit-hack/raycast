#include "map.h"
#include "things.h"
#include "pfields.h"
#include "spatial.h"

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
  state = IDLE;
}

void thing_imp_t::replan() {

  const int32_t px = int(pos.x);
  const int32_t py = int(pos.y);

  const uint8_t h = service.map->getHeight(px, py);
  const uint8_t p = service.pfield->get(px, py);

  const int32_t dx[] = { -1, 1,  0, 0 };
  const int32_t dy[] = {  0, 0, -1, 1 };

  int32_t tx = 0;
  int32_t ty = 0;

  uint8_t best_np = 0;

  for (int i = 0; i < 4; ++i) {
    const uint8_t nh = service.map->getHeight(px+dx[i], py+dy[i]);
    const uint8_t np = service.pfield->get(px+dx[i], py+dy[i]);
    // lets pick the best one
    if (np < best_np) {
      continue;
    }
    // if its less good square
    if (np < p) {
      continue;
    }
    // if inpassable
    if (nh > h + 1) {
      continue;
    }
    tx = dx[i];
    ty = dy[i];
  }

  if (tx != 0 || ty != 0) {
    target.x = px + tx + .5f + .25f * (randf() - .5f);
    target.y = py + ty + .5f + .25f * (randf() - .5f);
  }
}

void thing_imp_t::on_damage(
  const vec3f_t &src,
  const vec3f_t &hit,
  float damage) {

  state = HURT;
  _timer = 0;

  const vec3f_t d = normal(src - pos);
  acc = acc - d * damage;

  service.particles->spawn(hit, SPRITE_GORE, rand(), 9);
}

void thing_imp_t::tick_idle() {
  const int32_t px = int(pos.x);
  const int32_t py = int(pos.y);
  const uint8_t p = service.pfield->get(px, py);
  if (p > 0x68) {
    state = WALKING;
    _timer = 0;
    target = vec2f_t{pos.x, pos.y};
  }
}

void thing_imp_t::tick_walking() {

  ++_timer;
  if (_timer > 100) {
    replan();
    _timer = 0;
  }

  const float dx = target.x - pos.x;
  const float dy = target.y - pos.y;
  dir = atan2f(dy, dx);

  const float d = dx*dx + dy*dy;
  if (d < .2f) {

    const vec3f_t pp = player_pos();

    if (service.spatial->LOS(pos + vec3f_t{0.f, 0.f, 3.f}, pp + vec3f_t{0.f, 0.f, 3.f})) {
      state = SHOOTING;
      _timer = 0;
    }

    replan();
    _timer = 0;
  }
  else {
    acc.x += move_to(dx, 0.003f, 0.01f);
    acc.y += move_to(dy, 0.003f, 0.01f);
  }
}

void thing_imp_t::tick_shooting() {
  ++_timer;
  if (_timer == 10) {
    // so some shooting!
  }
  if (_timer > 30) {
    target = vec2f_t{ pos.x, pos.y };
    state = WALKING;
    _timer = 0;
  }
}

void thing_imp_t::tick_hurt() {
  ++_timer;
  if (_timer > 15) {
    target = vec2f_t{ pos.x, pos.y };
    state = WALKING;
    _timer = 0;
  }
}

void thing_imp_t::tick_dead() {
}

void thing_imp_t::tick_falling() {
  const uint8_t h = service.map->getHeight(int(pos.x), int(pos.y));
  if (pos.z <= h) {
    state = WALKING;
  }
}

void thing_imp_t::tick() {

  switch (state) {
  default:
  case IDLE:     tick_idle();     break;
  case WALKING:  tick_walking();  break;
  case SHOOTING: tick_shooting(); break;
  case HURT:     tick_hurt();     break;
  case DEAD:     tick_dead();     break;
  case FALLING:  tick_falling();  break;
  }

  vec3f_t new_pos = pos + acc;

  const uint8_t h = service.map->getHeight(int(pos.x), int(pos.y));
  if (new_pos.z <= h) {
    new_pos.z = h;
    acc.z = 0.f;
  }
  else {
    acc.z -= gravity;
    state = FALLING;
    _timer = 0;
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

  draw();
}

void thing_imp_t::draw() {
  const uint8_t l = service.map->getLight(int(pos.x), int(pos.y));

  const vec3f_t &p = player_pos();
  // direction from player to imp
  const vec3f_t d = pos - p;
  const float angle = atan2f(d.y, d.x);

  const float frame = 4.5f + (((angle - dir) / (pi * 2)) * 8.f);

  uint32_t f = int32_t(frame) & 0x7;

  draw_sprite(sprites[0], pos, height, l, f, 0);
}

thing_t *thing_create_imp() {
  return new thing_imp_t;
}

bool thing_imp_t::screen_aabb(vec2f_t &min, vec2f_t &max) const {

  sprite_t &s = sprites[SPRITE_IMP];

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
