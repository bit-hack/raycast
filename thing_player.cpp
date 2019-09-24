#include "map.h"
#include "things.h"
#include "pfields.h"

// speed modifiers
const float moveSpeed = 0.5f / ticks_per_sec;
const float rotSpeed  = 2.7f / ticks_per_sec;

void thing_player_t::on_create() {
  eyeLevel = 3.f;
  viewBob = 0.f;
}

void thing_player_t::tick() {
  do_movement();
}

void thing_player_t::do_movement() {

#if 0
  {
    SDL_ShowCursor(SDL_DISABLE);

    SDL_GrabMode(SDL_GRAB_ON);
    int32_t mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_WarpMouse(screen_w, screen_h);

    const int32_t dx = mx - screen_w;
    dir += float(dx) * 0.006f;
  }
#endif

  const uint8_t *keys = SDL_GetKeyState(nullptr);
  const vec2f_t dirVec = {sinf(dir), cosf(dir)};

  // integrate player position
  pos.x += acc.x;
  pos.y += acc.y;

  // dampen
  acc.x *= 0.9f;
  acc.y *= 0.9f;

  // move forward if no wall in front of you
  if (keys[SDLK_UP] || keys[SDLK_w]) {
    acc.x += dirVec.x * moveSpeed;
    acc.y += dirVec.y * moveSpeed;
  }
  // move backwards if no wall behind you
  if (keys[SDLK_DOWN] || keys[SDLK_s]) {
    acc.x += -dirVec.x * moveSpeed;
    acc.y += -dirVec.y * moveSpeed;
  }
  if (keys[SDLK_a]) {
    acc.x -= dirVec.y * moveSpeed;
    acc.y += dirVec.x * moveSpeed;
  }
  if (keys[SDLK_d]) {
    acc.x += dirVec.y * moveSpeed;
    acc.y -= dirVec.x * moveSpeed;
  }
  // rotate to the right
  if (keys[SDLK_RIGHT]) {
    dir += rotSpeed;
  }
  // rotate to the left
  if (keys[SDLK_LEFT]) {
    dir -= rotSpeed;
  }

  // resolve player collisions
  {
    vec2f_t res = {0.f, 0.f};
    map.resolve(pos, .3f, res);
    pos.x += res.x * .5f;
    pos.y += res.y * .5f;
  }

  // do gravity and floor collision checking
  const float fl = float(map.getHeight(int(pos.x), int(pos.y)));
  if (pos.z <= fl) {
    pos.z = fl;
    acc.z = 0.f;
  } else {
    acc.z -= 0.1f;
    pos.z += acc.z;
  }

  // lerp the eye level so it doesn't pop
  {
    const float pi = 3.14159265359f;
    viewBob += 2.f * pi / ticks_per_sec;
    if (viewBob > 2.f * pi) {
      viewBob -= 2.f * pi;
    }
    float acc_mag = sqrtf(vec3f_t::dot(acc, acc)) * 2.25f;
    float bob = sinf(viewBob) * acc_mag;
    eyeLevel += 0.2f * ((pos.z + bob + 3.f) - eyeLevel);
  }

  // 
  draw_sprite(sprites[2], vec2f_t{0, 0}, 255, 0);
}

thing_t *thing_create_player() {
  return new thing_player_t;
}

thing_player_t &player() {
  auto &p = service.things->get_things(PLAYER);
  thing_t *t = p.front();
  return *(thing_player_t*)t;
}

vec3f_t &player_pos() {
  return player().pos;
}

float player_eye_level() {
  return player().eyeLevel;
}

vec2f_t player_dir() {
  const float &dir = player().dir;
  return vec2f_t{sinf(dir), cosf(dir)};
}

float player_angle() {
  auto &p = service.things->get_things(PLAYER);
  thing_t *t = p.front();
  return t->dir;
}
