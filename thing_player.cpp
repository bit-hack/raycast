#include "common.h"
#include "map.h"
#include "things.h"
#include "pfields.h"
#include "spatial.h"

// speed modifiers
const float moveSpeed = 0.5f / ticks_per_sec;
const float rotSpeed  = 2.7f / ticks_per_sec;

const uint32_t num_gun_frames = 4;

void thing_player_t::on_create() {
  eyeLevel = 32.f;
  viewBob = 0.f;
  accMag = 0.f;
  gunFrame = 0.f;
}

void thing_player_t::do_shoot() {

  vec3f_t hit;
  thing_t *thing = nullptr;
  const vec2f_t &dir = player_dir();
  service.spatial->hitscan(pos.x, pos.y, dir.x, dir.y, hit, thing);
  if (thing) {

    thing->on_damage(pos, hit, .1f);

    vec2f_t out;
    project(hit, out);
    for (int y = out.y - 4; y < out.y + 4; ++y) {
      for (int x = out.x - 4; x < out.x + 4; ++x) {
        if ((x ^ y) & 1) {
          plot(x, y, 0xFF00FF);
        }
      }
    }
  }

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
  accMag = std::min(1.f, sqrtf(vec3f_t::dot(acc, acc)));

  // kick off a shot
  if (gunFrame == 0.f) {
    if (keys[SDLK_LCTRL]) {
      do_shoot();
      gunFrame = 1.f;
    }
  }

  do_movement();
  draw_gun();
}

void thing_player_t::draw_gun() {

  // if animation has started then run it
  gunFrame += (gunFrame > 0.f) ? 0.3f : 0.f;
  if (gunFrame >= num_gun_frames) {
    gunFrame = 0.f;
  }
  const uint32_t frame = uint32_t(gunFrame);

  // draw weapon
  const float wx =       sinf(viewBob)  * accMag * 64.f;
  const float wy = fabsf(cosf(viewBob)) * accMag * 64.f;
  const uint8_t light = service.map->getLight(int32_t(pos.x), int32_t(pos.y));
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
  vec3f_t new_pos = pos + acc;
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
    map.resolve(new_pos, .3f, res);
    new_pos.x += res.x * .5f;
    new_pos.y += res.y * .5f;
  }

  // do gravity and floor collision checking
  const float fl = float(map.getHeight(int(pos.x), int(pos.y)));
  if (new_pos.z <= fl) {
    new_pos.z = fl;
    acc.z = 0.f;
  } else {
    acc.z -= 0.1f;
  }

  // lerp the eye level so it doesn't pop
  {
    const float bob = sinf(viewBob) * accMag * 2.25f;
    eyeLevel += 0.2f * ((pos.z + bob + height) - eyeLevel);
  }

  // move to new location
  move(new_pos);
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
