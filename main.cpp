#define _SDL_main_h
#include <SDL/SDL.h>

#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <array>

#include "common.h"
#include "vector.h"
#include "texture.h"
#include "map.h"


enum axis_t { axis_x, axis_y };

// player
vec3f_t player_pos{22.f, 12.f, 0.f};
vec3f_t player_acc{0.f, 0.f, 0.f};
float player_dir = 0.f;
float eyeLevel = 3.f;
const float nearPlaneDist = .66f;


SDL_Surface *surf;
std::array<uint32_t, w*h> screen;
std::array<uint16_t, w*h> depth;


static void draw_floor(
  int32_t x,
  float miny,
  float y0,
  float y1,
  const vec2f_t &p0,
  const vec2f_t &p1,
  const float dist)
{
  const int32_t drawStart = SDL_max(int32_t(SDL_min(miny, y0)), 0);
  const int32_t drawEnd   = SDL_min(int32_t(SDL_min(miny, y1)), h - 1);
  if (drawStart >= drawEnd) {
    return;
  }

  // XXX: find p0.5 midpoint and project this to generate y0.5 a curve

  uint32_t level = 0;
  level += (dist > 3.f ? 1 : 0);
  level += (dist > 5.f ? 1 : 0);
  level += (dist > 8.f ? 1 : 0);
  level += (dist > 16.f ? 1 : 0);

  uint32_t tex_mask = 0x3f >> level;
  uint32_t tex_size = 64 >> level;
  const uint32_t *tex = texture[1].getTexture(level);

  const float dy = y1 - y0;
  const vec2f_t step{(p0.x - p1.x) / dy, (p0.y - p1.y) / dy};

  vec2f_t f = p1;

  uint32_t *p = screen.data();
  p += x;
  p += drawStart * w;

  uint16_t *d = depth.data();
  d += x;
  d += drawStart * w;
  const uint16_t dval = uint16_t(dist * 256.f);

  for (int32_t y = drawStart; y < drawEnd; ++y) {

    const uint32_t cx = uint32_t(f.x * tex_size) & tex_mask;
    const uint32_t cy = uint32_t(f.y * tex_size) & tex_mask;
    const uint32_t index =  (uint32_t(f.x * tex_size) & tex_mask) |
                           ((uint32_t(f.y * tex_size) & tex_mask) * tex_size);
    *p = tex[index];
    *d = dval;

    d += w;
    p += w;
    f = f + step;
  }
}

static void draw_wall(
  int32_t x,
  float miny,
  float y1,
  float y2,
  int32_t height,
  int32_t oldheight,
  axis_t axis,
  const vec2f_t &isect,
  const float dist) {

  const int32_t drawStart = SDL_max(int32_t(SDL_min(miny, y1)), 0);
  const int32_t drawEnd = SDL_min(int32_t(SDL_min(miny, y2)), h - 1);

  uint32_t level = 0;
  level += (dist > 3.f ? 1 : 0);
  level += (dist > 6.f ? 1 : 0);
  level += (dist > 10.f ? 1 : 0);
  level += (dist > 18.f ? 1 : 0);

  uint32_t tex_mask = 0x3f >> level;
  uint32_t tex_size = 64   >> level;
  const uint32_t *tex = texture[0].getTexture(level);

  const float dy = float(height - oldheight) / (y1 - y2);

  const uint32_t u = uint32_t(((axis == axis_x) ? isect.y : isect.x) * tex_size);

  // adjust if top of wall might extend above screen
  float v = -dy * (y1 < 0.f ? fabsf(y1) : 0.f);
  tex += u & tex_mask;

  uint32_t *p = screen.data();
  p += x;
  p += drawStart * w;

  uint16_t *d = depth.data();
  d += x;
  d += drawStart * w;
  const uint16_t dval = uint16_t(dist * 256.f);

  for (int32_t y = drawStart; y < drawEnd; ++y) {
    *p = tex[ tex_size * (uint32_t(v * tex_size / 4) & tex_mask) ];
    *d = dval;
    p += w;
    d += w;
    v -= dy;
  }
}

void raycast(
  uint32_t x,
  float vx, float vy,
  float rx, float ry) {

  // which grid cell we are in
  vec2i_t cell = {int32_t(vx), int32_t(vy)};

  // length between axis strides
  vec2f_t delta = { fabsf(1.f / rx), fabsf(1.f / ry) };

  // axis step
  const vec2i_t step = {(rx >= 0) ? 1 : -1, (ry >= 0) ? 1 : -1};

  // length accumulator
  vec2f_t len = { delta.x * ((rx < 0) ? fpart(vx) : 1.f - fpart(vx)),
                  delta.y * ((ry < 0) ? fpart(vy) : 1.f - fpart(vy)) };

  // axis travel per grid cell
  const vec2f_t dd = { rx / fabsf(ry), ry / fabsf(rx) };

  // starting point for x axis intersections
  vec2f_t px;
  px.x = (rx > 0) ? ipart(vx) : ipart(1.f + vx);
  px.y = vy + (px.x - vx) * (ry / rx);

  // starting point for y axis intersections
  vec2f_t py;
  py.y = (ry > 0) ? ipart(vy) : ipart(1.f + vy);
  py.x = vx + (py.y - vy) * (rx / ry);

  // perpendicular ray distance
  float pdist = 0.f;

  float miny = h;
  float oldy = h;
  uint8_t oldHeight = map.getHeight(cell.x, cell.y);

  vec2f_t isect0 = {vx, vy};
  vec2f_t isect1;

  axis_t axis = axis_x;
  while (true) {

    // step ray to next intersection
    if (len.x < len.y) {
      axis = axis_x;
      // step ray
      len.x += delta.x;
      cell.x += step.x;
      // step intersection point
      px += vec2f_t { float(step.x), dd.y };
      isect1 = px;
      // calculate perp distance
      pdist = ((cell.x - vx) + (step.x < 0 ? 1 : 0)) / rx;

    } else {
      axis = axis_y;
      // step ray
      len.y += delta.y;
      cell.y += step.y;
      // step intersection point
      py += vec2f_t { dd.x, float(step.y) };
      isect1 = py;
      // calculate perp distance
      pdist = ((cell.y - vy) + (step.y < 0 ? 1 : 0)) / ry;
    }

    // current floor height
    const uint8_t height = map.getHeight(cell.x, cell.y);

    // draw floor tile
    {
      const float newy = project(oldHeight, pdist);
      draw_floor(x, miny, newy, oldy, isect0, isect1, pdist);
      oldy = newy;
      miny = SDL_min(newy, miny);
    }

    // draw wall
    if (height > oldHeight) {
      const float y0 = project(height, pdist);
      const float y1 = project(oldHeight, pdist);
      draw_wall(x, miny, y0, y1, height, oldHeight, axis, isect1, pdist);
      oldy = y0;
      miny = SDL_min(y0, miny);
    }

    oldHeight = height;

    // check map tile for collision
    if (height >= 9) {
      break;
    }

    // swap intersection points
    isect0 = isect1;
  }
}

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
  if (keys[SDLK_q]) {
    eyeLevel += 0.03f;
  }
  if (keys[SDLK_a]) {
    eyeLevel -= 0.03f;
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
  eyeLevel += 0.1f * ((player_pos.z + 3.f) - eyeLevel);
}

void present(void) {
#if 0
  const uint32_t *src = screen.data();
  uint32_t *dst = (uint32_t*)surf->pixels;
  const uint32_t pitch = surf->pitch / 4;
  for (uint32_t y = 0; y < h; ++y) {
    for (uint32_t x = 0; x < w; ++x) {
      const uint32_t rgb = src[x];
      dst[x * 2 + 0] = rgb;
      dst[x * 2 + 1] = rgb;
      dst[x * 2 + 0 + pitch] = rgb;
      dst[x * 2 + 1 + pitch] = rgb;
    }
    src += w;
    dst += pitch * 2;
  }
#else
  const uint16_t *src = depth.data();
  uint32_t *dst = (uint32_t*)surf->pixels;
  const uint32_t pitch = surf->pitch / 4;
  for (uint32_t y = 0; y < h; ++y) {
    for (uint32_t x = 0; x < w; ++x) {
      const uint32_t rgb = (src[x] & 0xff00) * 8;
      dst[x * 2 + 0] = rgb;
      dst[x * 2 + 1] = rgb;
      dst[x * 2 + 0 + pitch] = rgb;
      dst[x * 2 + 1 + pitch] = rgb;
    }
    src += w;
    dst += pitch * 2;
  }
#endif
}

#define DIR_ROOT "/home/aidan/projects/raycast"
//#define DIR_ROOT "C:/repos/loderay"

bool load_textures() {
  texture[0].load(DIR_ROOT "/data/walls/boxy.bmp");
  texture[1].load(DIR_ROOT "/data/floors/hex.bmp");
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

  surf = SDL_SetVideoMode(w * 2, h * 2, 32, 0);

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
      const float planeX = dir.y * nearPlaneDist;
      const float planeY = -dir.x * nearPlaneDist;

      // for screen width
      for (int x = 0; x < w; x++) {
        // calculate ray direction
        const float cameraX = 2.f * x / float(w) - 1.f;
        const float rayDirX = dir.x + planeX * cameraX;
        const float rayDirY = dir.y + planeY * cameraX;
        raycast(x, player_pos.x, player_pos.y, rayDirX, rayDirY);
      }
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
