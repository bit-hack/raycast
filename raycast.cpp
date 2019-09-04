#include "raycast.h"


enum axis_t { axis_x, axis_y };

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
  const int32_t drawEnd   = SDL_min(int32_t(SDL_min(miny, y1)), screen_h - 1);
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
  p += drawStart * screen_w;

  uint16_t *d = depth.data();
  d += x;
  d += drawStart * screen_w;
  const uint16_t dval = uint16_t(dist * 256.f);

  for (int32_t y = drawStart; y < drawEnd; ++y) {

    const uint32_t cx = uint32_t(f.x * tex_size) & tex_mask;
    const uint32_t cy = uint32_t(f.y * tex_size) & tex_mask;
    const uint32_t index =  (uint32_t(f.x * tex_size) & tex_mask) |
                           ((uint32_t(f.y * tex_size) & tex_mask) * tex_size);
    *p = tex[index];
    *d = dval;

    d += screen_w;
    p += screen_w;
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
  const int32_t drawEnd = SDL_min(int32_t(SDL_min(miny, y2)), screen_h - 1);

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
  p += drawStart * screen_w;

  uint16_t *d = depth.data();
  d += x;
  d += drawStart * screen_w;
  const uint16_t dval = uint16_t(dist * 256.f);

  for (int32_t y = drawStart; y < drawEnd; ++y) {
    *p = tex[ tex_size * (uint32_t(v * tex_size / 4) & tex_mask) ];
    *d = dval;
    p += screen_w;
    d += screen_w;
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

  float miny = screen_h;
  float oldy = screen_h;
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
