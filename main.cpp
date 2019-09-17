#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#define _SDL_main_h
#include "SDL.h"

#include "common.h"
#include "map.h"
#include "raycast.h"
#include "sprites.h"
#include "texture.h"
#include "vector.h"
#include "things.h"
#include "pfields.h"

// player
vec3f_t player_pos{60.f, 66.f, 24.f};
vec3f_t player_acc{0.f, 0.f, 0.f};
float player_dir = 0.f;
float eye_level = 3.f;
const float near_plane_scale = .66f;
const uint32_t ticks_per_sec = 30;
float view_bob = 0.f;

// service locator
service_t service;

static SDL_Surface *surf;

bool load_assets() {
// The asset files are currently loaded relative to the executable.
#define DIR_ROOT "."

  texture[0].load(DIR_ROOT "/data/walls/boxy.bmp");
  texture[1].load(DIR_ROOT "/data/floors/hex.bmp");
  texture[2].load(DIR_ROOT "/data/ceil/stone.bmp");
  texture[3].load(DIR_ROOT "/data/floors/slime.bmp");
  texture[4].load(DIR_ROOT "/data/walls/kbrick1.bmp");
  texture[5].load(DIR_ROOT "/data/floors/fstna_2.bmp");

  sprites[0].load(DIR_ROOT "/data/things/test.bmp");
  sprites[1].load(DIR_ROOT "/data/sky/sky.bmp");

  map.load(DIR_ROOT "/data/map/e1m1");
  return true;
}

static void mouse_look() {
#if 0
  SDL_ShowCursor(SDL_DISABLE);

  SDL_GrabMode(SDL_GRAB_ON);
  int32_t mx, my;
  SDL_GetMouseState(&mx, &my);
  SDL_WarpMouse(screen_w, screen_h);

  const int32_t dx = mx - screen_w;
  player_dir += float(dx) * 0.006f;
#endif
}

static void player_move(float moveSpeed, float rotSpeed) {
  const uint8_t *keys = SDL_GetKeyState(nullptr);

  const vec2f_t dir = {sinf(player_dir), cosf(player_dir)};

  // integrate player position
  player_pos.x += player_acc.x;
  player_pos.y += player_acc.y;

  // dampen
  player_acc.x *= 0.9f;
  player_acc.y *= 0.9f;

  // move forward if no wall in front of you
  if (keys[SDLK_UP] || keys[SDLK_w]) {
    player_acc.x += dir.x * moveSpeed;
    player_acc.y += dir.y * moveSpeed;
  }
  // move backwards if no wall behind you
  if (keys[SDLK_DOWN] || keys[SDLK_s]) {
    player_acc.x += -dir.x * moveSpeed;
    player_acc.y += -dir.y * moveSpeed;
  }
  if (keys[SDLK_a]) {
    player_acc.x -= dir.y * moveSpeed;
    player_acc.y += dir.x * moveSpeed;
  }
  if (keys[SDLK_d]) {
    player_acc.x += dir.y * moveSpeed;
    player_acc.y -= dir.x * moveSpeed;
  }
  // rotate to the right
  if (keys[SDLK_RIGHT]) {
    player_dir += rotSpeed;
  }
  // rotate to the left
  if (keys[SDLK_LEFT]) {
    player_dir -= rotSpeed;
  }
  if (keys[SDLK_ESCAPE]) {
    SDL_Quit();
  }

  // resolve player collisions
  {
    vec2f_t res = {0.f, 0.f};
    map.resolve(player_pos, 0.3f, res);
    player_pos.x += res.x * 0.5f;
    player_pos.y += res.y * 0.5f;
  }

  // do gravity and floor collision checking
  const float fl = float(map.getHeight(int(player_pos.x), int(player_pos.y)));
  if (player_pos.z < fl) {
    player_pos.z = fl;
    player_acc.z = 0.f;
  } else {
    player_acc.z -= 0.1f;
    player_pos.z += player_acc.z;
  }

  // lerp the eye level so it doesn't pop
  {
    const float pi = 3.14159265359f;
    view_bob += 2.f * pi / ticks_per_sec;
    if (view_bob > 2.f * pi) {
      view_bob -= 2.f * pi;
    }
    float acc_mag = sqrtf(vec3f_t::dot(player_acc, player_acc)) * 2.25f;
    float bob = sinf(view_bob) * acc_mag;
    eye_level += 0.1f * ((player_pos.z + bob + 3.f) - eye_level);
  }

  // update pfield
  service.pfield->set(player_pos.x, player_pos.y, 0x75);
}

bool active = true;

void on_event(const SDL_Event *event) {
  if (event->type == SDL_QUIT) {
    active = false;
  }
  if (event->type == SDL_KEYDOWN) {
    switch (event->key.keysym.sym) {
    case SDLK_ESCAPE:
      active = false;
      break;
    case SDLK_F10:
      printf("%f, %f, %f\n", player_pos.x, player_pos.y, player_pos.z);
      break;
    case SDLK_F12:
      load_assets();
      break;
    }
  }
}

void redraw(void) {
  const vec2f_t dir = {sinf(player_dir), cosf(player_dir)};
  const float planeX = dir.y * near_plane_scale;
  const float planeY = -dir.x * near_plane_scale;

  // for screen width
  for (int x = 0; x < screen_w; x++) {
    // calculate ray direction
    const float cameraX = 2.f * x / float(screen_w) - 1.f;
    const float rayDirX = dir.x + planeX * cameraX;
    const float rayDirY = dir.y + planeY * cameraX;
    raycast(x, player_pos.x, player_pos.y, rayDirX, rayDirY);
  }
}

void draw_pfield() {
  pfield_t *p = service.pfield;
  for (uint32_t y = 0; y < map_h; ++y) {
    for (uint32_t x = 0; x < map_w; ++x) {
      const uint8_t v = p->get(x, y);
      lightmap[x + y * screen_w] = 0xff;
      depth[x + y * screen_w] = 0;
      screen[x + y * screen_w] = v;
    }
  }
}

void tick(void) {

  service.pfield->update();

  // speed modifiers
  const float moveSpeed = 0.5f / ticks_per_sec;
  const float rotSpeed = 2.7f / ticks_per_sec;

  player_move(moveSpeed, rotSpeed);
  mouse_look();

  redraw();

  service.things->tick();

//  draw_pfield();

  // present the screen
  present_screen_sse(surf);
  SDL_Flip(surf);
  screen.fill(0x10);
}

int main(int argc, char *args[]) {

  SDL_Init(SDL_INIT_VIDEO);

  if (!load_assets()) {
    return 1;
  }

  thing_manager_t things;

  service.map = &map;
  service.things = &things;
  service.pfield = new pfield_t(map);

  thing_t *t = service.things->create(IMP);
  t->pos = vec3f_t{63.2f, 64.8f, 16};

  thing_t *k = service.things->create(IMP);
  k->pos = vec3f_t{61.f, 64.3f, 16};

  surf = SDL_SetVideoMode(screen_w * 2, screen_h * 2, 32, 0);
  if (!surf) {
    return 2;
  }

  uint32_t old_ticks = SDL_GetTicks();

  while (active) {

    // pump SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      on_event(&event);
    }

    // timing for input and FPS counter
    const uint32_t new_ticks = SDL_GetTicks();
    const uint32_t tick_thresh = 1000 / ticks_per_sec;
    const uint32_t tick_diff = new_ticks - old_ticks;
    if (tick_diff > 1000) {
      old_ticks = new_ticks;
      continue;
    }
    if (tick_diff < tick_thresh) {
      SDL_Delay(1);
      continue;
    }
    old_ticks += tick_diff;

    // update
    tick();
  }
}
