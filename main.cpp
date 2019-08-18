/*
Copyright (c) 2004-2007, Lode Vandevenne

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define _SDL_main_h
#include <SDL/SDL.h>

#include <cmath>
#include <string>
#include <vector>
#include <iostream>

/*
g++ *.cpp -lSDL -O3 -W -Wall -ansi -pedantic
g++ *.cpp -lSDL
*/

//place the example code below here:

#define mapWidth 24
#define mapHeight 24

int worldMap[mapWidth][mapHeight]=
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

const int w = 512;
const int h = 384;

SDL_Surface *surf;

//draw the pixels of the stripe as a vertical line
void verLine(int x, int drawStart, int drawEnd, uint32_t color) {
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

uint32_t wallColour(int wall) {
  switch (wall) {
  case 1:
    return 0xff0000;
  case 2:
    return 0x00ff00;
  case 3:
    return 0x0000ff;
  case 4:
    return 0xffffff;
  default:
    return 0x00ffff;
  }
}

void raycast(int x, float cameraX, float posX, float posY, float rayDirX,
             float rayDirY) {
  // which box of the map we're in
  int mapX = int(posX);
  int mapY = int(posY);

  // length of ray from current position to next x or y-side
  float sideDistX;
  float sideDistY;

  // length of ray from one x or y-side to next x or y-side
  float deltaDistX = std::abs(1.f / rayDirX);
  float deltaDistY = std::abs(1.f / rayDirY);
  float perpWallDist;

  // what direction to step in x or y-direction (either +1 or -1)
  int stepX;
  int stepY;

  int hit = 0; // was there a wall hit?
  int side;    // was a NS or a EW wall hit?
  // calculate step and initial sideDist
  if (rayDirX < 0) {
    stepX = -1;
    sideDistX = (posX - mapX) * deltaDistX;
  } else {
    stepX = 1;
    sideDistX = (mapX + 1.0f - posX) * deltaDistX;
  }
  if (rayDirY < 0) {
    stepY = -1;
    sideDistY = (posY - mapY) * deltaDistY;
  } else {
    stepY = 1;
    sideDistY = (mapY + 1.0f - posY) * deltaDistY;
  }
  // perform DDA
  while (hit == 0) {
    // jump to next map square, OR in x-direction, OR in y-direction
    if (sideDistX < sideDistY) {
      sideDistX += deltaDistX;
      mapX += stepX;
      side = 0;
    } else {
      sideDistY += deltaDistY;
      mapY += stepY;
      side = 1;
    }
    // Check if ray has hit a wall
    if (worldMap[mapX][mapY] > 0)
      hit = 1;
  }
  // Calculate distance projected on camera direction (Euclidean distance will
  // give fisheye effect!)
  if (side == 0)
    perpWallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
  else
    perpWallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;

  // Calculate height of line to draw on screen
  int lineHeight = (int)(h / perpWallDist);

  // calculate lowest and highest pixel to fill in current stripe
  int drawStart = -lineHeight / 2 + h / 2;
  int drawEnd   =  lineHeight / 2 + h / 2;

  // choose wall color
  int color = wallColour(worldMap[mapX][mapY]);

  // give x and y sides different brightness
  if (side == 1) {
    color = (color >> 1) & 0x7f7f7f7f;
  }

  // draw the pixels of the stripe as a vertical line
  verLine(x, drawStart, drawEnd, color);
}

int main(int argc, char *args[])
{
  float posX = 22, posY = 12;  //x and y start position
  float dirX = -1, dirY = 0; //initial direction vector
  float planeX = 0, planeY = 0.66f; //the 2d raycaster version of camera plane

  uint32_t time = 0; //time of current frame
  uint32_t oldTime = 0; //time of previous frame

  SDL_Init(SDL_INIT_VIDEO);
  surf = SDL_SetVideoMode(w, h, 32, 0);

  bool done = false;

  while(!done)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        done = true;
        break;
      }
    }

    for(int x = 0; x < w; x++)
    {
      // calculate ray position and direction
      float cameraX = 2 * x / float(w) - 1; // x-coordinate in camera space
      float rayDirX = dirX + planeX * cameraX;
      float rayDirY = dirY + planeY * cameraX;

      raycast(x, cameraX, posX, posY, rayDirX, rayDirY);
    }

    //timing for input and FPS counter
    oldTime = time;
    time = SDL_GetTicks();
    float frameTime = (time - oldTime) / 1000.f; //frameTime is the time this frame has taken, in seconds

    SDL_Flip(surf);
    SDL_FillRect(surf, nullptr, 0x101010);

    //speed modifiers
    float moveSpeed = frameTime * 5.0f; //the constant value is in squares/second
    float rotSpeed = frameTime * 3.0f; //the constant value is in radians/second

    const uint8_t *keys = SDL_GetKeyState(nullptr);

    //move forward if no wall in front of you
    if (keys[SDLK_UP])
    {
      if(worldMap[int(posX + dirX * moveSpeed)][int(posY)] == false) posX += dirX * moveSpeed;
      if(worldMap[int(posX)][int(posY + dirY * moveSpeed)] == false) posY += dirY * moveSpeed;
    }
    //move backwards if no wall behind you
    if (keys[SDLK_DOWN])
    {
      if(worldMap[int(posX - dirX * moveSpeed)][int(posY)] == false) posX -= dirX * moveSpeed;
      if(worldMap[int(posX)][int(posY - dirY * moveSpeed)] == false) posY -= dirY * moveSpeed;
    }
    //rotate to the right
    if (keys[SDLK_RIGHT])
    {
      //both camera direction and camera plane must be rotated
      float oldDirX = dirX;
      dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
      dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
      float oldPlaneX = planeX;
      planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
      planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    }
    //rotate to the left
    if (keys[SDLK_LEFT])
    {
      //both camera direction and camera plane must be rotated
      float oldDirX = dirX;
      dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
      dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
      float oldPlaneX = planeX;
      planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
      planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
    }
  }
}
