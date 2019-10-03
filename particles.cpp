#include "sprites.h"
#include "particles.h"


void particle_manager_t::tick() {
  for (auto itt = list.begin(); itt != list.end();) {
    auto &p = *itt;
    if (--p.age <= 0) {
      itt = list.erase(itt);
      continue;
    }

    auto &s = sprites[p.sprite];

#if 0
    vec2f_t spos;
    if (project(p.pos, spos)) {
      draw_sprite(s, spos, 0xff, 0);
    }
#else
    vec3f_t pos = p.pos;
    const float height = s.frame_h / 12;
//    pos.z -= height / 2;
    draw_sprite(s, pos, height, 0xff, p.index);
#endif

    ++itt;
  }
}

void particle_manager_t::spawn(const vec3f_t &pos, int8_t sprite, int32_t index, int32_t age) {

  printf("spawning %f %f %f\n", pos.x, pos.y, pos.z);

  auto &s = sprites[sprite];
  const uint32_t frames = s.h / s.frame_h;

  list.emplace_back();
  auto &p = list.back();
  p.pos = pos;
  p.sprite = sprite;
  p.age = age;
  p.index = index % frames;
  p.acc = vec3f_t{0.f, 0.f, 0.f};
}
