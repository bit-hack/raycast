#include "map.h"
#include "things.h"
#include "pfields.h"

// speed modifiers
const float moveSpeed = 0.5f / ticks_per_sec;
const float rotSpeed  = 2.7f / ticks_per_sec;

const uint32_t num_gun_frames = 4;

void thing_player_t::on_create() {
  eyeLevel = 3.f;
  viewBob = 0.f;
}

void thing_player_t::tick() {

  const uint8_t *keys = SDL_GetKeyState(nullptr);

  // update viewbob accumulator
  {
    const float pi = 3.14159265359f;
    viewBob += 2.f * pi / ticks_per_sec;
    if (viewBob > 2.f * pi) {
      viewBob -= 2.f * pi;
    }
  }

  // acceleration magnitude
  const float accMag = std::min(1.f, sqrtf(vec3f_t::dot(acc, acc)));

  // kick off a shot
  if (gun_frame == 0) {
    if (keys[SDLK_LCTRL]) {
      gun_frame = ticks_per_sec / num_gun_frames;
    }
  }

  do_movement();
  draw_gun();
}

void thing_player_t::draw_gun() {

  // if animation has started then run it
  gun_frame += (gun_frame > 0);
  if (gun_frame >= ticks_per_sec) {
    gun_frame = 0;
  }
  const uint32_t frame = (gun_frame * num_gun_frames) / ticks_per_sec;

  // draw weapon
  const float wx =       sinf(viewBob)  * accMag * 64.f;
  const float wy = fabsf(cosf(viewBob)) * accMag * 64.f;
  const uint8_t light = service.map->getLight(float(pos.x), float(pos.y));
  draw_sprite(sprites[2], vec2f_t{148 + wx, 90 + wy}, light, frame);
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
    const float bob = sinf(viewBob) * accMag * 2.25f;
    eyeLevel += 0.2f * ((pos.z + bob + 3.f) - eyeLevel);
  }
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
