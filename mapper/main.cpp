#define _CRT_SECURE_NO_WARNINGS
#define _SDL_main_h
#include "SDL.h"

#include <array>
#include <vector>


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

  std::vector<uint32_t*> textures;

  void log_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    fputs("\n", stdout);
    va_end(args);
  }

  uint32_t *load_texture(const char *path) const {
    uint32_t *dst = new uint32_t[64 * 64];
    uint32_t *out = dst;
    SDL_Surface *bmp = SDL_LoadBMP(path);
    if (!bmp) {
      return nullptr;
    }
    if (bmp->w != 64 || bmp->h != 64 || bmp->format->BitsPerPixel != 24) {
      SDL_FreeSurface(bmp);
      return nullptr;
    }
    const uint8_t *src = (const uint8_t *)bmp->pixels;
    for (uint32_t y = 0; y < 64; ++y) {
      for (uint32_t x = 0; x < 64; ++x) {
        const uint32_t r = src[x * 3 + 2];
        const uint32_t g = src[x * 3 + 1];
        const uint32_t b = src[x * 3 + 0];
        dst[x] = (r << 16) | (g << 8) | b;
      }
      src += 64 * 3;
      dst += 64;
    }
    SDL_FreeSurface(bmp);
    return out;
  }

  bool load_textures() {
    FILE *f = fopen("data/textures.txt", "r");
    if (!f) {
      return true;
    }
    std::array<char, 256> temp;
    while (!feof(f)) {
      temp[0] = '\0';
      if (!fgets(temp.data(), temp.size(), f)) {
        break;
      }
      temp[255] = '\0';
      temp[strcspn(temp.data(), "\n" )] = '\0';
      uint32_t *tex = load_texture(temp.data());
      if (tex) {
        textures.push_back(tex);
      }
      else {
        log_printf("error loading texture '%s'", temp.data());
      }
    }
    fclose(f);
    return true;
  }

  app_t() {
    draw_plane = PLANE_FLOOR;
    write_mode = WRITE_PIXEL;
    write_value = 64;
    context = CONTEXT_MAP;
    highlight_value = -1;
    hover_value = -1;
    level_filename = "./data/map/e1m1.map";
    draw_toggle = false;
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
    load_textures();
  }

  void draw_sidebar_greyscale() {
    const uint32_t w = sidebar_w / 2;
    SDL_Rect rect = {512, 0, w, 512 / 64};
    for (int y = 0; y < 512; y += rect.h) {
      rect.x = 512;
      rect.y = y;
      const uint32_t t = (y / rect.h);
      const uint32_t rgb0 = 0x5588AA;
      const uint32_t rgb1 = (t << 18) | (t << 10) | (t << 2);
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

  void draw_sidebar_texture() {
    // texture selector
    for (size_t i = 0; i < textures.size(); ++i) {
      if ((i * 64) > (screen_h - 64)) {
        break;
      }
      uint32_t *dst = (uint32_t*)surf->pixels;
      dst += screen_w;
      dst += (surf->pitch / 4) * i * 64;
      uint32_t *src = textures[i];
      for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
          dst[x] = src[x];
        }
        dst += (surf->pitch / 4);
        src += 64;
      }
    }
  }

  void draw_sidebar() {
    switch (draw_plane) {
    case 0:
    case 1:
    case 2:
      // greyscale selector
      draw_sidebar_greyscale();
      break;
    case 3:
    case 4:
    case 5:
      // texture selector
      draw_sidebar_texture();
      break;
    }
  }

  void draw_map_greyscale() {
    const auto &p = plane[draw_plane];
    const uint8_t *src = p.data();
    SDL_Rect rect = {0, 0, cell_w, cell_h};
    for (int y = 0; y < map_h; ++y) {
      for (int x = 0; x < map_w; ++x) {
        const uint8_t t = src[x];
        const uint32_t rgb = (t << 18) | (t << 10) | (t << 2);
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

  void draw_map_texture() {
    const auto &p = plane[draw_plane];
    uint32_t *dst = (uint32_t*)surf->pixels;
    for (int y = 0; y < screen_h; ++y) {
      for (int x = 0; x < screen_w; ++x) {
        const int cx = x / cell_w;
        const int cy = y / cell_h;
        const int ci = cx + cy * map_w;
        const uint8_t val = p[ci];
        if (val >= textures.size()) {
          dst[x] = 0xff00ff;
        }
        else {
          const uint32_t *tex = textures[val];
          dst[x] = tex[(x & 0x3f) + (y & 0x3f) * 64];
        }
      }
      dst += surf->pitch / 4;
    }
  }

  void draw_map_combine() {
    const auto &p = plane[draw_plane];
    const auto &h = plane[0];

    uint32_t *dst = (uint32_t*)surf->pixels;
    for (int y = 0; y < screen_h; ++y) {
      for (int x = 0; x < screen_w; ++x) {
        const int cx = x / cell_w;
        const int cy = y / cell_h;
        const int ci = cx + cy * map_w;
        const uint8_t val = p[ci];

        if (x & 1) {
          if (val >= textures.size()) {
            dst[x] = 0xff00ff;
          } else {
            const uint32_t *tex = textures[val];
            dst[x] = tex[(x & 0x3f) + (y & 0x3f) * 64];
          }
        }
        else {
          const uint8_t t = h[cx + cy * map_w];
          const uint32_t rgb = (t << 18) | (t << 10) | (t << 2);
          dst[x] = rgb;
        }
      }
      dst += surf->pitch / 4;
    }
  }

  void draw() {
    if (draw_toggle) {
      draw_map_combine();
    }
    else {
      switch (draw_plane) {
      case 0:
      case 1:
      case 2:
        draw_map_greyscale();
        break;
      case 3:
      case 4:
      case 5:
        draw_map_texture();
        break;
      }
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

  void on_mouse_button_down_map(SDL_Event &event) {
    const uint32_t x = (event.button.x / cell_w) & (map_w-1);
    const uint32_t y = (event.button.y / cell_h) & (map_h-1);
    // write value
    if (event.button.button == SDL_BUTTON_LEFT) {
      plane_write(draw_plane, x, y, write_value);
    }
    // sample value
    if (event.button.button == SDL_BUTTON_RIGHT) {
      write_value = plane_read(draw_plane, x, y);
      log_printf("write value: %d", (int)write_value);
    }
  }

  void on_mouse_button_down_sidebar(SDL_Event &event) {
    switch (draw_plane) {
    case 0:
    case 1:
    case 2:
      // greyscale selector
      if (event.button.button == SDL_BUTTON_LEFT) {
        write_value = sidebar_value(event.button.y);
        log_printf("write value: %d", (int)write_value);
      }
      break;
    case 3:
    case 4:
    case 5:
      // texture selector
      if (event.button.button == SDL_BUTTON_LEFT) {
        write_value = event.button.y / 64;
        log_printf("write value: %d", (int)write_value);
      }
    }
  }

  void on_mouse_button_down(SDL_Event &event) {
    if (event.button.state != SDL_PRESSED) {
      return;
    }
    if (event.button.x < screen_w) {
      on_mouse_button_down_map(event);
    }
    if (event.button.x >= screen_w) {
      on_mouse_button_down_sidebar(event);
    }
  }

  uint32_t sidebar_value(int y) const {
    return ((y / (screen_h / 64)));
  }

  void on_mouse_motion(SDL_Event &event) {
    const uint32_t x = (event.motion.x / cell_w) & (map_w-1);
    const uint32_t y = (event.motion.y / cell_h) & (map_h-1);
    if (event.motion.x < screen_w) {
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

  void on_load_level() {
    log_printf("loading level '%s'", level_filename.c_str());
    FILE *fd = fopen(level_filename.c_str(), "rb");
    if (!fd) {
      log_printf("unable to fopen file");
      return;
    }
    for (auto &p : plane) {
      size_t read = fread(p.data(), 1, p.size(), fd);
      if (read != p.size()) {
        log_printf("error loading plane");
        break;
      }
    }
    fclose(fd);
  }

  void on_save_level() {
    log_printf("saving level '%s'", level_filename.c_str());
    FILE *fd = fopen(level_filename.c_str(), "wb");
    if (!fd) {
      log_printf("unable to fopen file");
      return;
    }
    for (auto &p : plane) {
      size_t written = fwrite(p.data(), 1, p.size(), fd);
      if (written != p.size()) {
        log_printf("error writing plane");
        break;
      }
    }
    fclose(fd);
  }

  const char *draw_plane_name() {
    switch (draw_plane) {
    case 0:  return "floor";
    case 1:  return "ceil";
    case 2:  return "light";
    case 3:  return "tex floor";
    case 4:  return "tex ceil";
    case 5:  return "tex wall";
    default: return "";
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
      log_printf("draw plane: %s", draw_plane_name());
      break;
    }
    case SDLK_b:
      log_printf("write mode: pixel");
      write_mode = WRITE_PIXEL;
      break;
    case SDLK_g:
      log_printf("write mode: fill");
      write_mode = WRITE_FILL;
      break;
    case SDLK_F5:
      on_save_level();
      break;
    case SDLK_F9:
      on_load_level();
      break;
    case SDLK_TAB:
      draw_toggle = !draw_toggle;
      log_printf("draw mode toggle %s", draw_toggle ? "on" : "off");
      break;
    case SDLK_UP:
      write_value -= (write_value > 0);
      break;
    case SDLK_DOWN:
      write_value += (write_value < 63);
      break;
    }
  }

  int main(const int argc, char **args) {
    if (argc >= 2) {
      level_filename = args[1];
    }
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

  bool draw_toggle;

  // current level filename
  std::string level_filename;

  // value on the sidebar the mouse is over
  int32_t highlight_value;
  // value on the map the mouse is over
  int32_t hover_value;

  context_t context;
};

int main(const int argc, char **args) {
  app_t *app = new app_t;
  return app->main(argc, args);
}
