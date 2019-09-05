#include "raycast.h"


enum axis_t { axis_x, axis_y };


static void draw_ceil(
  int32_t x,
  float miny,
  float maxy,
  float y0,
  float y1,
  const vec2f_t &p0,
  const vec2f_t &p1,
  const float d0,
  const float d1) {

  const int32_t drawStart = SDL_max(int32_t(y0), maxy);
  const int32_t drawEnd   = SDL_min(int32_t(SDL_min(miny, y1)), screen_h - 1);
  if (drawStart >= drawEnd) {
    return;
  }

  uint32_t level = 0;
  level += (d1 >  3.f ? 1 : 0);
  level += (d1 >  5.f ? 1 : 0);
  level += (d1 >  8.f ? 1 : 0);
  level += (d1 > 16.f ? 1 : 0);

  uint32_t tex_mask = 0x3f >> level;
  uint32_t tex_size = 64 >> level;
  const uint32_t *tex = texture[2].getTexture(level);

  // note: work from p1 -> p0 because we render downwards

  const float dy = y1 - y0;

  uint32_t *p = screen.data();
  p += x;
  p += drawStart * screen_w;

  uint16_t *d = depth.data();
  d += x;
  d += drawStart * screen_w;
  const uint16_t dval = uint16_t(d1 * 256.f);

  const float w0 = 1.f / d0;
  const float w1 = 1.f / d1;
  const float dw = (w1 - w0) / dy;

  const float u0 = p0.x / d0;
  const float u1 = p1.x / d1;
  const float du = (u1 - u0) / dy;

  const float v0 = p0.y / d0;
  const float v1 = p1.y / d1;
  const float dv = (v1 - v0) / dy;

  const float nudge = drawStart > y0 ? drawStart - y0 : 0;

  float u = u0 + du * nudge;
  float v = v0 + dv * nudge;
  float w = w0 + dw * nudge;

  for (int32_t y = drawStart; y < drawEnd; ++y) {

    const float pu = u / w;
    const float pv = v / w;

    const uint32_t index =  (uint32_t(pu * tex_size) & tex_mask) |
                           ((uint32_t(pv * tex_size) & tex_mask) * tex_size);
    u += du;
    v += dv;
    w += dw;

    *p = tex[index];
    *d = 256.f / w;

    d += screen_w;
    p += screen_w;
  }
}

static void draw_floor(
  int32_t x,
  float miny,
  float maxy,
  float y0,
  float y1,
  const vec2f_t &p0,
  const vec2f_t &p1,
  const float d0,
  const float d1)
{
  const int32_t drawStart = SDL_max(int32_t(y0), maxy);
  const int32_t drawEnd   = SDL_min(int32_t(SDL_min(miny, y1)), screen_h - 1);
  if (drawStart >= drawEnd) {
    return;
  }

  uint32_t level = 0;
  level += (d1 >  3.f ? 1 : 0);
  level += (d1 >  5.f ? 1 : 0);
  level += (d1 >  8.f ? 1 : 0);
  level += (d1 > 16.f ? 1 : 0);

  uint32_t tex_mask = 0x3f >> level;
  uint32_t tex_size = 64 >> level;
  const uint32_t *tex = texture[1].getTexture(level);

  // note: work from p1 -> p0 because we render downwards

  const float dy = y1 - y0;

  uint32_t *p = screen.data();
  p += x;
  p += drawStart * screen_w;

  uint16_t *d = depth.data();
  d += x;
  d += drawStart * screen_w;
  const uint16_t dval = uint16_t(d1 * 256.f);

  const float w0 = 1.f / d0;
  const float w1 = 1.f / d1;
  const float dw = (w0 - w1) / dy;

  const float u0 = p0.x / d0;
  const float u1 = p1.x / d1;
  const float du = (u0 - u1) / dy;

  const float v0 = p0.y / d0;
  const float v1 = p1.y / d1;
  const float dv = (v0 - v1) / dy;

  float u = u1;
  float v = v1;
  float w = w1;

  for (int32_t y = drawStart; y < drawEnd; ++y) {

    const float pu = u / w;
    const float pv = v / w;

    const uint32_t index =  (uint32_t(pu * tex_size) & tex_mask) |
                           ((uint32_t(pv * tex_size) & tex_mask) * tex_size);
    u += du;
    v += dv;
    w += dw;

    *p = tex[index];
    *d = 256.f / w;

    d += screen_w;
    p += screen_w;
  }
}

static void draw_step_down(
  int32_t x,
  float miny,
  float maxy,
  float y1,
  float y2,
  int32_t ceil,
  int32_t oldCeil,
  axis_t axis,
  const vec2f_t &isect,
  const float dist)
{
  const int32_t drawStart = SDL_max(int32_t(SDL_max(maxy, y1)), 0);
  const int32_t drawEnd = SDL_min(int32_t(SDL_min(miny, y2)), screen_h - 1);

  uint32_t level = 0;
  level += (dist > 4.f ? 1 : 0);
  level += (dist > 7.f ? 1 : 0);
  level += (dist > 13.f ? 1 : 0);
  level += (dist > 20.f ? 1 : 0);

  uint32_t tex_mask = 0x3f >> level;
  uint32_t tex_size = 64 >> level;
  const uint32_t *tex = texture[0].getTexture(level);

  const float dy = float(ceil - oldCeil) / (y1 - y2);

  const uint32_t u =
      uint32_t(((axis == axis_x) ? isect.y : isect.x) * tex_size);

  // adjust if top of wall might extend above screen
  float v = (drawStart > y1) ? -(dy * (drawStart - y1)) : 0;
  tex += u & tex_mask;

  uint32_t *p = screen.data();
  p += x;
  p += drawStart * screen_w;

  uint16_t *d = depth.data();
  d += x;
  d += drawStart * screen_w;
  const uint16_t dval = uint16_t(dist * 256.f);

  for (int32_t y = drawStart; y < drawEnd; ++y) {
    *p = tex[tex_size * (uint32_t(v * tex_size / 4) & tex_mask)];
    *d = dval;
    p += screen_w;
    d += screen_w;
    v -= dy;
  }
}

static void draw_step_up(
  int32_t x,
  float miny,
  float maxy,
  float y1,
  float y2,
  int32_t height,
  int32_t oldheight,
  axis_t axis,
  const vec2f_t &isect,
  const float dist) {

  const int32_t drawStart = SDL_max(int32_t(SDL_max(maxy, y1)), 0);
  const int32_t drawEnd = SDL_min(int32_t(SDL_min(miny, y2)), screen_h - 1);

  uint32_t level = 0;
  level += (dist > 4.f ? 1 : 0);
  level += (dist > 7.f ? 1 : 0);
  level += (dist > 13.f ? 1 : 0);
  level += (dist > 20.f ? 1 : 0);

  uint32_t tex_mask = 0x3f >> level;
  uint32_t tex_size = 64 >> level;
  const uint32_t *tex = texture[0].getTexture(level);

  const float dy = float(height - oldheight) / (y1 - y2);

  const uint32_t u =
      uint32_t(((axis == axis_x) ? isect.y : isect.x) * tex_size);

  // adjust if top of wall might extend above screen
  float v = (drawStart > y1) ? -(dy * (drawStart - y1)) : 0;
  tex += u & tex_mask;

  uint32_t *p = screen.data();
  p += x;
  p += drawStart * screen_w;

  uint16_t *d = depth.data();
  d += x;
  d += drawStart * screen_w;
  const uint16_t dval = uint16_t(dist * 256.f);

  for (int32_t y = drawStart; y < drawEnd; ++y) {
    *p = tex[tex_size * (uint32_t(v * tex_size / 4) & tex_mask)];
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
  float dist = 0.f;

  float miny = screen_h;
  float oldminy = screen_h;
  uint8_t oldFloor = map.getHeight(cell.x, cell.y);

  float maxy = 0;
  float oldmaxy = 0;
  uint8_t oldCeil = map.getCeil(cell.x, cell.y);

  vec2f_t isect0 = {vx, vy};
  vec2f_t isect1;

  axis_t axis = axis_x;
  while (true) {

    const float old_dist = dist;

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
      dist = ((cell.x - vx) + (step.x < 0 ? 1 : 0)) / rx;

    } else {
      axis = axis_y;
      // step ray
      len.y += delta.y;
      cell.y += step.y;
      // step intersection point
      py += vec2f_t { dd.x, float(step.y) };
      isect1 = py;
      // calculate perp distance
      dist = ((cell.y - vy) + (step.y < 0 ? 1 : 0)) / ry;
    }

    // current floor/ceiling height
    const uint8_t floor = map.getHeight(cell.x, cell.y);
    const uint8_t ceil = map.getCeil(cell.x, cell.y);

    // draw floor tile
    {
      const float y = project(oldFloor, dist);
      draw_floor(x, miny, maxy, y, oldminy, isect0, isect1, old_dist, dist);
      oldminy = y;
      miny = SDL_min(y, miny);
    }

    // draw ceiling tile
    {
      const float y = project(oldCeil, dist, 0.f);
      draw_ceil(x, miny, maxy, oldmaxy, y, isect0, isect1, old_dist, dist);
      oldmaxy = y;
      maxy = SDL_max(y, maxy);
    }

    // draw step down
    if (ceil < oldCeil) {
      const float y0 = project(oldCeil, dist, 0.f);
      const float y1 = project(ceil, dist, 0.f);
      draw_step_down(x, miny, maxy, y0, y1, oldCeil, ceil, axis, isect1, dist);
      oldmaxy = y1;
      maxy = SDL_max(y1, maxy);
    }

    // draw step up
    if (floor > oldFloor) {
      const float y0 = project(floor, dist);
      const float y1 = project(oldFloor, dist);
      draw_step_up(x, miny, maxy, y0, y1, floor, oldFloor, axis, isect1, dist);
      oldminy = y0;
      miny = SDL_min(y0, miny);
    }

    oldFloor = floor;
    oldCeil = ceil;

    // check map tile for collision
    if (floor >= 9) {
      break;
    }

    // swap intersection points
    isect0 = isect1;
  }
}
