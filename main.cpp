#define _SDL_main_h
#include <SDL/SDL.h>

#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <array>

template <typename type_t>
struct vec2_t {
  type_t x, y;

  vec2_t operator + (const vec2_t &a) const {
    return vec2_t{a.x + x, a.y + y};
  }
};

using vec2f_t = vec2_t<float>;
using vec2i_t = vec2_t<int32_t>;

enum axis_t { axis_x, axis_y };

#define mapWidth  24
#define mapHeight 24

uint8_t worldMap[mapWidth][mapHeight] = {
  {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9},
  {9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,9},
  {9,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,1,0,3,0,0,0,9},
  {9,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,9},
  {9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,0,0,0,0,0,1,2,3,4,5,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,0,0,0,0,0,1,2,3,4,5,0,0,0,1,1,1,0,0,0,0,0,0,9},
  {9,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,9},
  {9,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,9},
  {9,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,4,0,0,0,0,7,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9}
};

enum {
  w = 320,
  h = 240,
};

float playerX = 22.f;
float playerY = 12.f;

float playerDX = -1;
float playerDY =  0;

float eyeLevel = 3.f;
float nearPlaneDist = .66f;

SDL_Surface *surf;
std::array<uint32_t, w*h> screen;

static float fpart(float x) {
  return x - float(int32_t(x));
}

static float ipart(float x) {
  return float(int32_t(x));
}

static float project(float y, float dist) {
  if (dist < 0.01f) {
    return float(h);
  }
  else {
    return (h / 2.f) - 64 * (y - eyeLevel) / dist;
  }
}

static void draw_floor(int32_t x, float miny, float y1, float y2, const vec2f_t &p0, const vec2f_t &p1) {
  const int32_t drawStart = SDL_max(int32_t(SDL_min(miny, y1)), 0);
  const int32_t drawEnd   = SDL_min(int32_t(SDL_min(miny, y2)), h - 1);
  if (drawStart >= drawEnd) {
    return;
  }

  const float dy = y2 - y1;
  const vec2f_t step{(p0.x - p1.x) / dy, (p0.y - p1.y) / dy};

  vec2f_t f = p1;

  uint32_t *p = screen.data();
  p += x;
  p += drawStart * w;
  for (int32_t y = drawStart; y < drawEnd; ++y) {

    const uint32_t cx = uint32_t(f.x * 256) & 0xff;
    const uint32_t cy = uint32_t(f.y * 256) & 0xff;

    *p = cx | (cy << 8);

    p += w;

    f = f + step;
  }
}

static void draw_wall(int32_t x, float miny, float y1, float y2, int32_t height,
                      int32_t oldheight, axis_t axis, const vec2f_t &isect,
                      uint32_t rgb) {

  const int32_t drawStart = SDL_max(int32_t(SDL_min(miny, y1)), 0);
  const int32_t drawEnd = SDL_min(int32_t(SDL_min(miny, y2)), h - 1);

  const float dy = 256.f * float(height - oldheight) / (y1 - y2);

  const uint32_t fr = uint32_t(((axis == axis_x) ? isect.y : isect.x) * 256.f);
  rgb = (fr & 0xff) << 8;

  float v = 0.f;

  uint32_t *p = screen.data();
  p += x;
  p += drawStart * w;
  for (int32_t y = drawStart; y < drawEnd; ++y) {

    *p = rgb | (uint32_t(v) & 0xff);
    p += w;
    v += dy;
  }
}

void raycast(
  uint32_t x,
  float px, float py,
  float rx, float ry) {

  // which grid cell we are in
  vec2i_t cell = {int32_t(px), int32_t(py)};

  // length between axis strides
  const float xdelta = fabsf(1.f / rx);  // x stride
  const float ydelta = fabsf(1.f / ry);  // y stride

  // axis step
  const vec2i_t step = {(rx >= 0) ? 1 : -1, (ry >= 0) ? 1 : -1};

  // length accumulator
  float xlen = xdelta * ((rx < 0) ? fpart(px) : 1.f - fpart(px));
  float ylen = ydelta * ((ry < 0) ? fpart(py) : 1.f - fpart(py));

  // axis travel per grid cell
  const float ddx = rx / fabsf(ry);
  const float ddy = ry / fabsf(rx);

  // starting point for x axis intersections
  float pxx = (rx > 0) ? ipart(px) : ipart(1.f + px);
  const float pdx = pxx - px;
  float pxy = py + pdx * (ry / rx);

  // starting point for y axis intersections
  float pyy = (ry > 0) ? ipart(py) : ipart(1.f + py);
  const float pdy = pyy - py;
  float pyx = px + pdy * (rx / ry);

  // perpendicular ray distance
  float pdist = 0.f;

  float miny = h;
  float oldy = h;
  float oldHeight = worldMap[cell.x][cell.y];

  vec2f_t isect0 = {px, py};
  vec2f_t isect1;

  axis_t axis = axis_x;
  while (true) {

    if (xlen < ylen) {
      axis = axis_x;
      // step ray
      xlen += xdelta;
      cell.x += step.x;
      // step intersection point
      pxy += ddy;
      pxx += step.x;
      isect1.x = pxx;
      isect1.y = pxy;
      // calculate perp distance
      const float distx = (cell.x - px) + (step.x < 0 ? 1 : 0);
      pdist = distx / rx;

    } else {
      axis = axis_y;
      // step ray
      ylen += ydelta;
      cell.y += step.y;
      // step intersection point
      pyx += ddx;
      pyy += step.y;
      isect1.x = pyx;
      isect1.y = pyy;
      // calculate perp distance
      const float disty = (cell.y - py) + (step.y < 0 ? 1 : 0);
      pdist = disty / ry;
    }

    const uint8_t height = worldMap[cell.x][cell.y];

    // draw floor tile
    {
      const float newy = project(oldHeight, pdist);
      draw_floor(x, miny, newy, oldy, isect0, isect1);
      oldy = newy;
      miny = SDL_min(newy, miny);
    }

    // draw wall
    if (height > oldHeight) {
      const float y0 = project(height, pdist);
      const float y1 = project(oldHeight, pdist);
      const uint32_t color = (axis == axis_x) ? 0xffffff : 0x7f7f7f;
      draw_wall(x, miny, y0, y1, height, oldHeight, axis, isect1, color);
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

  // move forward if no wall in front of you
  if (keys[SDLK_UP]) {
    if (worldMap[int(playerX + playerDX * moveSpeed)][int(playerY)] == 0)
      playerX += playerDX * moveSpeed;
    if (worldMap[int(playerX)][int(playerY + playerDY * moveSpeed)] == 0)
      playerY += playerDY * moveSpeed;
  }
  // move backwards if no wall behind you
  if (keys[SDLK_DOWN]) {
    if (worldMap[int(playerX - playerDX * moveSpeed)][int(playerY)] == 0)
      playerX -= playerDX * moveSpeed;
    if (worldMap[int(playerX)][int(playerY - playerDY * moveSpeed)] == 0)
      playerY -= playerDY * moveSpeed;
  }
  // rotate to the right
  if (keys[SDLK_RIGHT]) {
    // both camera direction and camera plane must be rotated
    const float oldDirX = playerDX;
    const float oldDirY = playerDY;
    playerDX = oldDirX * cosf(-rotSpeed) - oldDirY * sinf(-rotSpeed);
    playerDY = oldDirX * sinf(-rotSpeed) + oldDirY * cosf(-rotSpeed);
  }
  // rotate to the left
  if (keys[SDLK_LEFT]) {
    // both camera direction and camera plane must be rotated
    const float oldDirX = playerDX;
    const float oldDirY = playerDY;
    playerDX = oldDirX * cosf(rotSpeed) - oldDirY * sinf(rotSpeed);
    playerDY = oldDirX * sinf(rotSpeed) + oldDirY * cosf(rotSpeed);
  }
  if (keys[SDLK_q]) {
    eyeLevel += 0.03f;
  }
  if (keys[SDLK_a]) {
    eyeLevel -= 0.03f;
  }
}

void present(void) {
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
}

int main(int argc, char *args[])
{
  uint32_t oldTime = 0;

  SDL_Init(SDL_INIT_VIDEO);
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

    // for screen width
    for(int x = 0; x < w; x++)
    {
      // calculate ray direction
      const float planeX =  playerDY * nearPlaneDist;
      const float planeY = -playerDX * nearPlaneDist;
      const float cameraX = 2.f * x / float(w) - 1.f;
      const float rayDirX = playerDX + planeX * cameraX;
      const float rayDirY = playerDY + planeY * cameraX;
      raycast(x, playerX, playerY, rayDirX, rayDirY);
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
