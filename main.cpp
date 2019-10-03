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
#include "spatial.h"
#include "particles.h"

service_t service;
const float near_plane_scale = .66f;
static SDL_Surface *surf;

bool load_assets() {
  load_textures();
  load_sprites();
  map.load("./data/map/e1m1.map");
  return true;
}

bool active = true;

void on_event(const SDL_Event *event) {

  const vec3f_t &pos = player_pos();

  if (event->type == SDL_QUIT) {
    active = false;
  }
  if (event->type == SDL_KEYDOWN) {
    switch (event->key.keysym.sym) {
    case SDLK_ESCAPE:
      active = false;
      break;
    case SDLK_F10:
      printf("%f, %f, %f\n", pos.x, pos.y, pos.z);
      break;
    case SDLK_F12:
      load_assets();
      break;
    }
  }
}

void redraw(void) {

  const vec3f_t &pos = player_pos();

  const vec2f_t dir = player_dir();
  const float planeX = dir.y * near_plane_scale;
  const float planeY = -dir.x * near_plane_scale;

  // for screen width
  for (int x = 0; x < screen_w; x++) {
    // calculate ray direction
    const float cameraX = 2.f * x / float(screen_w) - 1.f;
    const float rayDirX = dir.x + planeX * cameraX;
    const float rayDirY = dir.y + planeY * cameraX;
    raycast(x, pos.x, pos.y, rayDirX, rayDirY);
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

void do_collisions(void) {
  std::set<std::pair<thing_t*, thing_t*>> pairs;
  service.spatial->overlaps(pairs);
  for (auto &p : pairs) {
    // TODO
  }
}

void tick(void) {
  // set player pfield
  const vec3f_t pos = player_pos();
  service.pfield->set(
    int32_t(pos.x),
    int32_t(pos.y), 0x75);
  service.pfield->update();
  // redraw raycasting
  redraw();
  // update all entities
  service.things->tick();
  // resolve collisions
  do_collisions();
  //
//  service.spatial->draw();

  service.particles->tick();

  // present the screen
  present_screen_sse(surf);
  SDL_Flip(surf);
  screen.fill(0x10);
}

void plot(int x, int y, uint32_t rgb) {
  if (x > 0 && x < screen_w) {
    if (y > 0 && y < screen_h) {
      screen[x + y * screen_w] = rgb;
    }
  }
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
  service.spatial = new spatial_t(map);
  service.particles = new particle_manager_t;

  thing_t *t = service.things->create(IMP);
  t->pos = vec3f_t{33.2f, 34.8f, 16};

  thing_t *k = service.things->create(IMP);
  k->pos = vec3f_t{34.f, 33.3f, 16};

  thing_t *p = service.things->create(PLAYER);
  p->pos = vec3f_t{32.f, 32.f};

  bool fullscreen = false;
  surf = SDL_SetVideoMode(screen_w * 2, screen_h * 2, 32, fullscreen ? SDL_FULLSCREEN : 0);
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
