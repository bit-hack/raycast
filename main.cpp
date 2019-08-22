#define _SDL_main_h
#include <SDL/SDL.h>

#include <cmath>
#include <string>
#include <vector>
#include <iostream>

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

const int w = 320;
const int h = 240;

float playerX = 22.f;
float playerY = 12.f;

float playerDX = -1;
float playerDY =  0;

float eyeLevel = 3.f;
float nearPlaneDist = .66f;

SDL_Surface *surf;

static void verLine(int x, int drawStart, int drawEnd, uint32_t color) {
  drawStart = SDL_max(drawStart, 0);
  drawEnd = SDL_min(drawEnd, h - 1);
  uint32_t *p = (uint32_t*)surf->pixels;
  p += x;
  p += drawStart * surf->w;
  for (int y = drawStart; y < drawEnd; ++y) {
    *p = color;
    p += surf->w;
  }
}

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

static void draw_vert(int32_t x, float y1, float y2, uint32_t rgb) {
  verLine(x, int32_t(y1), int32_t(y2), rgb);
}

void raycast(
  uint32_t x,
  float px, float py,
  float rx, float ry) {

  enum axis_t { axis_x, axis_y };

  // which grid cell we are in
  int intx = int32_t(px);
  int inty = int32_t(py);

  // length between axis strides
  const float xdelta = fabsf(1.f / rx);  // x stride
  const float ydelta = fabsf(1.f / ry);  // y stride

  // axis step
  const int32_t stepX = (rx >= 0) ? 1 : -1;
  const int32_t stepY = (ry >= 0) ? 1 : -1;

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

  float oldy = h;
  float oldHeight = 0.f;

  axis_t axis = axis_x;
  while (true) {

    if (xlen < ylen) {
      axis = axis_x;
      // step ray
      xlen += xdelta;
      intx += stepX;
      // step intersection point
      pxy += ddy;
      pxx += stepX;
      // calculate perp distance
      const float distx = (intx - px) + (stepX < 0 ? 1 : 0);
      pdist = distx / rx;

    } else {
      axis = axis_y;
      // step ray
      ylen += ydelta;
      inty += stepY;
      // step intersection point
      pyx += ddx;
      pyy += stepY;
      // calculate perp distance
      const float disty = (inty - py) + (stepY < 0 ? 1 : 0);
      pdist = disty / ry;
    }

    const uint8_t height = worldMap[intx][inty];

    float nexty = oldy;

    // project all edges as we need them for correct texture interpolation

    // draw floor tile
    {
      const float newy = project(SDL_max(oldHeight, height), pdist);
      draw_vert(x, newy, oldy, ((intx ^ inty) & 1) ? 0x202020 : 0x303030);
      nexty = SDL_min(oldy, newy);
    }

    // draw wall
    if (height > oldHeight) {
      const float y0 = project(oldHeight, pdist);
      const float y1 = project(height, pdist);
      const uint32_t color = (axis == axis_x) ? 0xffffff : 0x7f7f7f;
      draw_vert(x, y1, SDL_min(oldy, y0), color);
      nexty = SDL_min(oldy, y1);
    }

    oldy = nexty;
    oldHeight = height;

    // check map tile for collision
    if (height >= 9) {
      break;
    }
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
}

int main(int argc, char *args[])
{
  uint32_t oldTime = 0;

  SDL_Init(SDL_INIT_VIDEO);
  surf = SDL_SetVideoMode(w, h, 32, 0);

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
    SDL_Flip(surf);
    SDL_FillRect(surf, nullptr, 0x101010);
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
