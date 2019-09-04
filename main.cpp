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
vec3f_t player_pos{22.f, 12.f, 0.f};
vec3f_t player_acc{0.f, 0.f, 0.f};
float player_dir = 0.f;
float eye_level = 3.f;
const float near_plane_scale = .66f;


static SDL_Surface *surf;

std::array<uint32_t, screen_w*screen_h> screen;
std::array<uint16_t, screen_w*screen_h> depth;


static void doMove(float moveSpeed, float rotSpeed) {
  const uint8_t *keys = SDL_GetKeyState(nullptr);

  const vec2f_t dir = { sinf(player_dir), cosf(player_dir) };

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

void present(void) {
#if 1
  const uint32_t *src = screen.data();
  uint32_t *dst = (uint32_t*)surf->pixels;
  const uint32_t pitch = surf->pitch / 4;
  for (uint32_t y = 0; y < screen_h; ++y) {
    for (uint32_t x = 0; x < screen_w; ++x) {
      const uint32_t rgb = src[x];
      dst[x * 2 + 0] = rgb;
      dst[x * 2 + 1] = rgb;
      dst[x * 2 + 0 + pitch] = rgb;
      dst[x * 2 + 1 + pitch] = rgb;
    }
    src += screen_w;
    dst += pitch * 2;
  }
#else
  const uint16_t *src = depth.data();
  uint32_t *dst = (uint32_t*)surf->pixels;
  const uint32_t pitch = surf->pitch / 4;
  for (uint32_t y = 0; y < screen_h; ++y) {
    for (uint32_t x = 0; x < screen_w; ++x) {
      const uint32_t rgb = (src[x] * 8) & 0xff00;
      dst[x * 2 + 0] = rgb;
      dst[x * 2 + 1] = rgb;
      dst[x * 2 + 0 + pitch] = rgb;
      dst[x * 2 + 1 + pitch] = rgb;
    }
    src += screen_w;
    dst += pitch * 2;
  }
#endif
}

// The asset files are currently loaded relative to the executable.
#define DIR_ROOT "."

bool load_textures() {
  texture[0].load(DIR_ROOT "/data/walls/boxy.bmp");
  texture[1].load(DIR_ROOT "/data/floors/hex.bmp");
  sprites[0].load(DIR_ROOT "/data/things/test.bmp");
  return true;
}

const std::array<uint8_t, map_t::mapWidth * map_t::mapHeight> worldMap = {
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
  9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,9,
  9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,9,
  9,0,0,0,0,0,3,4,4,4,3,0,0,0,0,3,2,3,2,3,0,0,0,9,
  9,0,0,0,0,0,3,0,0,0,3,0,0,0,1,2,1,1,1,2,1,0,0,9,
  9,0,0,0,0,0,3,0,0,0,3,0,0,1,2,3,1,0,1,3,2,1,0,9,
  9,0,0,0,0,0,2,0,0,0,2,0,0,0,1,2,1,0,1,2,1,0,0,9,
  9,0,0,0,0,0,2,1,0,1,2,0,0,0,0,3,0,3,0,3,0,0,0,9,
  9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
  9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
  9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
  9,0,0,0,0,0,1,2,3,4,5,0,0,0,0,0,0,0,0,0,0,0,0,9,
  9,0,0,0,0,0,1,2,3,4,5,0,0,0,1,1,1,0,0,0,0,0,0,9,
  9,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,9,
  9,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,9,
  9,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
  9,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
  9,4,0,0,0,0,7,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
  9,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
  9,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
  9,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
  9,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9
};

int main(int argc, char *args[])
{
  uint32_t oldTime = 0;

  SDL_Init(SDL_INIT_VIDEO);

  if (!load_textures()) {
    return 1;
  }
  map.load(worldMap.data());

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
    {
      const std::array<vec3f_t, 8> points = {
        vec3f_t{12, 12, 1}, vec3f_t{12, 13, 1},
        vec3f_t{13, 12, 1}, vec3f_t{13, 13, 1},
        vec3f_t{12, 12, 2}, vec3f_t{12, 13, 2},
        vec3f_t{13, 12, 2}, vec3f_t{13, 13, 2},
      };
      vec2f_t p;
      float dist;
      for (const auto &j : points) {
        dist = project(j, p);
        if (dist >= 0.f) {
          if (p.x >= 0 && p.y >= 0 && p.x < screen_w && p.y < screen_h) {

            const int32_t index = int32_t(p.x) + int32_t(p.y) * screen_w;

            if (depth[index] < 256 * dist) {
              screen[index] = 0xff0000;
            }
            else {
              screen[index] = 0x00ff00;
            }
          }
        }
      }
      draw_sprite(sprites[0], vec3f_t{4, 4, 0}, 4);
    }

    // present the screen
    present();
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
