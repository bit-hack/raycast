#include <assert.h>

#include "spatial.h"


void spatial_t::insert(thing_t *t) {
  const int32_t minx = std::max(0,       int32_t(t->pos.x - t->radius));
  const int32_t miny = std::max(0,       int32_t(t->pos.y - t->radius));
  const int32_t maxx = std::min(map_w-1, int32_t(t->pos.x + t->radius));
  const int32_t maxy = std::min(map_h-1, int32_t(t->pos.y + t->radius));

  int32_t i = miny * map_w;

  for (int y = miny; y <= maxy; ++y) {
    for (int x = minx; x <= maxx; ++x) {
      auto &cell = cells[i + x];
      cell.push_back(t);
    }
    i += map_w;
  }
}

void spatial_t::remove(thing_t *t) {
  const int32_t minx = std::max(0,       int32_t(t->pos.x - t->radius));
  const int32_t miny = std::max(0,       int32_t(t->pos.y - t->radius));
  const int32_t maxx = std::min(map_w-1, int32_t(t->pos.x + t->radius));
  const int32_t maxy = std::min(map_h-1, int32_t(t->pos.y + t->radius));

  int32_t i = miny * map_w;

  for (int y = miny; y <= maxy; ++y) {
    for (int x = minx; x <= maxx; ++x) {

      // erase from cell
      auto &cell = cells[i + x];
      for (auto itt = cell.begin(); itt != cell.end(); ++itt) {
        if (*itt == t) {
          cell.erase(itt);
          break;
        }
      }

    }
    i += map_w;
  }
}

std::vector<thing_t*> & spatial_t::find(uint32_t x, uint32_t y) {
  assert(x >= 0 && x < map_w);
  assert(y >= 0 && y < map_h);
  return cells[x + y * map_w];
}

void spatial_t::hitscan(float vx, float vy, float rx, float ry, vec3f_t &hit, thing_t *&thing) {

  // XXX: add max distance
  // XXX: 3d hit location

  // which grid cell we are in
  vec2i_t cell = { int32_t(vx), int32_t(vy) };

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
  float maxy = 0;

  thing = nullptr;

  float best = 100000.f;

  uint8_t oldfloor = 0;
  uint8_t oldceil = 0;

  while (true) {

    // current floor height
    // we lag heights one tile because of flats
    const uint8_t newfloor = map.getHeight(cell.x, cell.y);
    const uint8_t floor = std::max(newfloor, oldfloor);
    oldfloor = newfloor;

    // current ceiling height
    // we lag heights one tile because of flats
    const uint8_t newceil = map.getCeil(cell.x, cell.y);
    const uint8_t ceil = std::min(newceil, oldceil);
    oldceil = newceil;

    // keep track of visible
    miny = std::min(miny, project(floor, dist, screen_h));
    maxy = std::max(maxy, project(ceil, dist, 0));

    // for all things in this cell
    for (auto &t : cells[cell.x + cell.y * map_w]) {
      vec2f_t min, max;
      if (!t->screen_aabb(min, max))
        continue;
      // if clipped and invisible
      if (max.y <= maxy)  // ceiling y
        continue;
      if (min.y >= miny)  // floor y
        continue;
      // check for x axis intersect
      const vec2f_t disp = vec2f_t{t->pos.x, t->pos.y} - vec2f_t{vx, vy};
      const float side_val = vec2f_t::dot(vec2f_t{ry, -rx}, disp);
      if ((side_val > t->radius) || (side_val < -t->radius)) {
        continue;
      }
      // check for closest thing
      const float dist = vec2f_t::dist_sqr(vec2f_t{vx, vy},
                                           vec2f_t{t->pos.x, t->pos.y});
      if (dist > best)
        continue;
      // we got our hit
      best = dist;
      thing = t;
      hit = t->pos;
      hit.z += t->height / 2;
      hit.x -= ry * side_val;
      hit.y += rx * side_val;
    }

    // we can early exit now if we got something
    if (thing) {
      break;
    }

    // step ray to next intersection
    if (len.x < len.y) {
      // step ray
      len.x += delta.x;
      cell.x += step.x;
      // step intersection point
      px += vec2f_t { float(step.x), dd.y };
      if (!thing) {
        hit = vec3f_t{px.x, px.y, 0.f};
      }
      // calculate perp distance
      dist = ((cell.x - vx) + (step.x < 0 ? 1 : 0)) / rx;

    } else {
      // step ray
      len.y += delta.y;
      cell.y += step.y;
      // step intersection point
      py += vec2f_t { dd.x, float(step.y) };
      if (!thing) {
        hit = vec3f_t{py.x, py.y, 0.f};
      }
      // calculate perp distance
      dist = ((cell.y - vy) + (step.y < 0 ? 1 : 0)) / ry;
    }

    // check map tile for collision
    if (floor >= 63 || (maxy >= miny)) {
      break;
    }
  }

  // if its a wall hit then keep the player z pos
  if (!thing) {
    hit.z = player_pos().z + 2.f;
  }
}

void spatial_t::overlaps(std::set<std::pair<thing_t*, thing_t*>> &out) {
  for (auto &c : cells) {
    if (c.size() <= 1) {
      continue;
    }
    for (size_t i = 0; i < c.size(); ++i) {
      for (size_t j = i + 1; j < c.size(); ++j) {

        thing_t *a = c[i];
        thing_t *b = c[j];

        const float r = a->radius + b->radius;
        const float rs = r * r;
        const vec3f_t d = b->pos - a->pos;
        const float ds = vec3f_t::dot(d, d);

        if (ds < rs) {
          if (a < b) {
            out.emplace(a, b);
          }
          else {
            out.emplace(b, a);
          }
        }
      }
    }
  }
}

void spatial_t::draw() {
  uint32_t* dst = screen.data();
  uint8_t* lit = lightmap.data();
  uint16_t *dpt = depth.data();
  auto *cell = cells.data();
  for (int y = 0; y < map_h; ++y) {
    for (int x = 0; x < map_w; ++x) {
      dst[x] = cell->size() * 64;
      lit[x] = 0xff;
      dpt[x] = 0x0;
      ++cell;
    }
    dst += screen_w;
  }
}
