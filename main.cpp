#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <array>

#define _SDL_main_h
#include "SDL.h"

#include "common.h"
#include "vector.h"
#include "texture.h"
#include "map.h"
#include "sprites.h"
#include "raycast.h"


// player
vec3f_t player_pos{60.f, 66.f, 24.f};
vec3f_t player_acc{0.f, 0.f, 0.f};
float player_dir = 0.f;
float eye_level = 3.f;
const float near_plane_scale = .66f;


static SDL_Surface *surf;


static void doMove(float moveSpeed, float rotSpeed) {
  const uint8_t *keys = SDL_GetKeyState(nullptr);

  const vec2f_t dir = { sinf(player_dir), cosf(player_dir) };

#if 0
  {
//    SDL_GrabMode(SDL_GRAB_ON);
    int32_t mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_WarpMouse(screen_w, screen_h);

    const int32_t dx = mx - screen_w;
    player_dir += float(dx) * 0.01f;
  }
#endif

  // integrate player position
  player_pos.x += player_acc.x;
  player_pos.y += player_acc.y;

  // dampen
  player_acc.x *= 0.95f;
  player_acc.y *= 0.95f;

  // move forward if no wall in front of you
  if (keys[SDLK_UP] || keys[SDLK_w]) {
    player_acc.x = dir.x * moveSpeed;
    player_acc.y = dir.y * moveSpeed;
  }
  // move backwards if no wall behind you
  if (keys[SDLK_DOWN] || keys[SDLK_s]) {
    player_acc.x = -dir.x * moveSpeed;
    player_acc.y = -dir.y * moveSpeed;
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
    vec2f_t res = { 0.f, 0.f };
    map.resolve(player_pos, 0.3f, res);
    player_pos.x += res.x * 0.5f;
    player_pos.y += res.y * 0.5f;
  }

  // do gravity and floor collision checking
  const float fl = float(map.getHeight(int(player_pos.x), int(player_pos.y)));
  if (player_pos.z < fl) {
    player_pos.z = fl;
    player_acc.z = 0.f;
  }
  else {
    player_acc.z -= 0.01f;
    player_pos.z += player_acc.z;
  }

  // lerp the eye level so it doesn't pop
  eye_level += 0.1f * ((player_pos.z + 3.f) - eye_level);
}

// The asset files are currently loaded relative to the executable.
#define DIR_ROOT "."

bool load_assets() {
  texture[0].load(DIR_ROOT "/data/walls/boxy.bmp");
  texture[1].load(DIR_ROOT "/data/floors/hex.bmp");
  texture[2].load(DIR_ROOT "/data/ceil/stone.bmp");
  sprites[0].load(DIR_ROOT "/data/things/test.bmp");
  map.load(DIR_ROOT "/data/map/e1m1_floor.bmp",
           DIR_ROOT "/data/map/e1m1_ceil.bmp");
  return true;
}

int main(int argc, char *args[])
{
  uint32_t oldTime = 0;

  SDL_Init(SDL_INIT_VIDEO);

  if (!load_assets()) {
    return 1;
  }

  surf = SDL_SetVideoMode(screen_w * 2, screen_h * 2, 32, 0);

  for(bool done = false; !done;)
  {
    // pump SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        done = true;
        break;
      }
      if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          done = true;
          break;
        }
      }
    }

    {
      const vec2f_t dir = {sinf(player_dir), cosf(player_dir)};
      const float planeX =  dir.y * near_plane_scale;
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

    // test projection
    draw_sprite(sprites[0], vec3f_t{4, 4, 0}, 4);

    // present the screen
    present_screen_sse(surf);
    SDL_Flip(surf);
    screen.fill(0x10);
    SDL_Delay(5);

    // timing for input and FPS counter
    const uint32_t time = SDL_GetTicks();
    const float frameTime = (time - oldTime) / 1000.f;
    oldTime = time;

    // speed modifiers
    const float moveSpeed = frameTime * 5.0f;
    const float rotSpeed = frameTime * 3.0f;

    doMove(moveSpeed, rotSpeed);
  }
}
