#define _SDL_main_h
#include <SDL/SDL.h>

#include <array>


struct app_t {

  enum context_t {
    CONTEXT_MAP,
    CONTEXT_SIDEBAR,
  };

  enum plane_t {
    PLANE_FLOOR = 0,
    PLANE_CEIL,
    PLANE_LIGHT,
    PLANE_TEX_FLOOR,
    PLANE_TEX_WALL,
    PLANE_TEX_CEIL,
  };

  enum write_t {
    WRITE_PIXEL = 0,
    WRITE_FILL
  };

  static const int screen_w  = 512;
  static const int screen_h  = 512;
  static const int map_w     = 64;
  static const int map_h     = 64;
  static const int cell_w    = screen_w / map_w;
  static const int cell_h    = screen_h / map_h;
  static const int sidebar_w = 64;

  bool active = false;

  app_t() {
    draw_plane = PLANE_FLOOR;
    write_mode = WRITE_PIXEL;
    write_value = 64;
    context = CONTEXT_MAP;
    highlight_value = -1;
    hover_value = -1;
  }

  bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      return false;
    }
    surf = SDL_SetVideoMode(screen_w + sidebar_w, screen_h, 32, 0);
    if (!surf) {
      return false;
    }
    return true;
  }

  void reset() {
    for (auto &p : plane) {
      p.fill(0);
    }
  }

  void draw_sidebar() {
    const uint32_t w = sidebar_w / 2;
    SDL_Rect rect = {512, 0, w, 512 / 64};
    for (int y=0; y<512; y += rect.h) {
      rect.x = 512;
      rect.y = y;
      const uint32_t t = ((y / rect.h) * 4);
      const uint32_t rgb0 = 0x5588AA;
      const uint32_t rgb1 = (t << 16) | (t << 8) | t;
      // value being written
      if (t == write_value) {
        SDL_FillRect(surf, &rect, rgb0);
      } else {
        SDL_FillRect(surf, &rect, rgb1);
      }
      // value your hovering over
      rect.x = 512 + w;
      if (t == hover_value && context == CONTEXT_MAP) {
        SDL_FillRect(surf, &rect, rgb0);
      } else {
        SDL_FillRect(surf, &rect, rgb1);
      }
    }
  }

  void draw() {
    const auto &p = plane[draw_plane];
    const uint8_t *src = p.data();
    SDL_Rect rect = {0, 0, cell_w, cell_h};
    for (int y = 0; y < map_h; ++y) {
      for (int x = 0; x < map_w; ++x) {
        const uint8_t t = src[x];
        const uint32_t rgb = (t << 16) | (t << 8) | t;
        rect.x = x * cell_w;
        rect.y = y * cell_h;
        if (context == CONTEXT_SIDEBAR) {
          SDL_FillRect(surf, &rect, (t == highlight_value) ? 0x5588AA : rgb);
        } else {
          SDL_FillRect(surf, &rect, rgb);
        }
      }
      src += map_w;
    }
  }

  void flood_fill(std::array<uint8_t, map_w * map_h> &p, int32_t x, int32_t y,
                  uint8_t value, uint8_t rep) const {
    if (value == rep) {
      return;
    }
    p[x + y * map_w] = value;
    if (x > 0) {
      if (p[(x - 1) + y * map_w] == rep) {
        flood_fill(p, x - 1, y, value, rep);
      }
    }
    if (x < (map_w - 1)) {
      if (p[(x + 1) + y * map_w] == rep) {
        flood_fill(p, x + 1, y, value, rep);
      }
    }
    if (y > 0) {
      if (p[x + (y - 1) * map_w] == rep) {
        flood_fill(p, x, y - 1, value, rep);
      }
    }
    if (y < (map_h - 1)) {
      if (p[x + (y + 1) * map_w] == rep) {
        flood_fill(p, x, y + 1, value, rep);
      }
    }
  }

  void plane_write(plane_t pl, uint32_t x, uint32_t y, uint8_t val) {
    if (write_mode == WRITE_PIXEL) {
      auto &p = plane[pl];
      uint8_t *dst = p.data();
      dst[x + y * map_w] = write_value;
    }
    if (write_mode == WRITE_FILL) {
      auto &p = plane[pl];
      const uint8_t rep = p[x + y * map_w];
      flood_fill(p, x, y, val, rep);
    }
  }

  uint8_t plane_read(plane_t pl, uint32_t x, uint32_t y) {
    auto &p = plane[pl];
    uint8_t *dst = p.data();
    return dst[x + y * map_w];
  }

  void on_mouse_button_down(SDL_Event &event) {
    if (event.button.state != SDL_PRESSED) {
      return;
    }
    const uint32_t x = (event.button.x / cell_w) & (map_w-1);
    const uint32_t y = (event.button.y / cell_h) & (map_h-1);
    if (event.button.x < 512) {
      // write value
      if (event.button.button == SDL_BUTTON_LEFT) {
        plane_write(draw_plane, x, y, write_value);
      }
      // sample value
      if (event.button.button == SDL_BUTTON_RIGHT) {
        write_value = plane_read(draw_plane, x, y);
      }
    }
    if (event.button.x >= 512) {
      if (event.button.button == SDL_BUTTON_LEFT) {
        write_value = ((event.button.y / (512 / 64))) * 4;
      }
    }
  }

  uint32_t sidebar_value(int y) const {
    return ((y / (512 / 64))) * 4;
  }

  void on_mouse_motion(SDL_Event &event) {
    const uint32_t x = (event.motion.x / cell_w) & (map_w-1);
    const uint32_t y = (event.motion.y / cell_h) & (map_h-1);
    if (event.motion.x < 512) {
      if (event.motion.state & SDL_BUTTON_LMASK) {
        plane_write(draw_plane, x, y, write_value);
      }
      context = CONTEXT_MAP;
      auto &p = plane[draw_plane];
      hover_value = p[x + y * map_w];
    }
    if (event.motion.x >= 512) {
      context = CONTEXT_SIDEBAR;
      highlight_value = sidebar_value(event.motion.y);
    }
  }

  void on_key_down(SDL_Event &event) {
    switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
      // kill the app
      active = false;
      break;
    case SDLK_1: // floor
    case SDLK_2: // ceil
    case SDLK_3: // light
    case SDLK_4: // tex floor
    case SDLK_5: // tex ceil
    case SDLK_6: // tex wall
    {
      // set plane
      const uint32_t index = event.key.keysym.sym - SDLK_1;
      draw_plane = plane_t(PLANE_FLOOR + index);
      break;
    }
    case SDLK_b:
      write_mode = WRITE_PIXEL;
      break;
    case SDLK_g:
      write_mode = WRITE_FILL;
      break;
    }
  }

  int main(const int argc, char **args) {
    if (!init()) {
      return 1;
    }
    reset();
    active = true;
    while (active) {
      SDL_FillRect(surf, nullptr, 0x101010);
      draw();
      draw_sidebar();
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
          active = false;
          break;
        case SDL_MOUSEBUTTONDOWN:
          on_mouse_button_down(event);
          break;
        case SDL_MOUSEMOTION:
          on_mouse_motion(event);
          break;
        case SDL_KEYDOWN:
          on_key_down(event);
          break;
        }
      }
      SDL_Flip(surf);
      SDL_Delay(2);
    }
    return 0;
  }

protected:
  std::array<std::array<uint8_t, map_w*map_h>, 6> plane;
  SDL_Surface *surf;
  plane_t draw_plane;
  write_t write_mode;
  uint8_t write_value;

  // 
  int32_t highlight_value;
  // 
  int32_t hover_value;

  context_t context;
};

int main(const int argc, char **args) {
  app_t *app = new app_t;
  return app->main(argc, args);
}
