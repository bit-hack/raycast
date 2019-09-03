#include "SDL.h"
#include "texture.h"


// mip level offsets
enum {
  mip0 = 0,
  mip1 = mip0 + 64*64,
  mip2 = mip1 + 32*32,
  mip3 = mip2 + 16*16,
  mip4 = mip3 + 8*8,
  mip5 = mip4 + 4*4,
  mip6 = mip5 + 2*2,
  mip7 = mip6 + 1*1,
};

static const std::array<uint32_t, 8> mip_offset = {
  mip0, mip1, mip2, mip3, mip4, mip5, mip6, mip7
};

const uint32_t *texture_t::getTexture(uint32_t level) const {
  return &texel[mip_offset[level]];
}

uint32_t *texture_t::_getTexture(uint32_t level) {
  return &texel[mip_offset[level]];
}

bool texture_t::load(const char *path) {
  SDL_Surface *bmp = SDL_LoadBMP(path);
  if (!bmp) {
    return false;
  }

  if (bmp->w != 64 || bmp->h != 64 || bmp->format->BitsPerPixel != 24) {
    SDL_FreeSurface(bmp);
    return false;
  }

  const uint8_t *src = (const uint8_t *)bmp->pixels;
  uint32_t *dst = _getTexture(0);

  for (uint32_t y = 0; y < 64; ++y) {
    for (uint32_t x = 0; x < 64; ++x) {
      const uint32_t r = src[x * 3 + 0];
      const uint32_t g = src[x * 3 + 1];
      const uint32_t b = src[x * 3 + 2];
      dst[x] = (r << 16) | (g << 8) | b;
    }
    src += 64 * 3;
    dst += 64;
  }

  SDL_FreeSurface(bmp);

  _genMips();
  return true;
}

void texture_t::_genMips(void) {
  for (uint32_t i = 1; i < 7; ++i) {
    const uint32_t size = 64 >> i;

    const uint32_t *src = _getTexture(i - 1);
    uint32_t *dst = _getTexture(i - 0);

    for (uint32_t y = 0; y < size; ++y) {
      for (uint32_t x = 0; x < size; ++x) {
        // grab texels
        const uint32_t t0 = src[(x * 2 + 0) + (y * 2 + 0) * size * 2];
        const uint32_t t1 = src[(x * 2 + 1) + (y * 2 + 0) * size * 2];
        const uint32_t t2 = src[(x * 2 + 0) + (y * 2 + 1) * size * 2];
        const uint32_t t3 = src[(x * 2 + 1) + (y * 2 + 1) * size * 2];
        // average channels together
        uint32_t r = (((t0 & 0xff0000) + (t1 & 0xff0000) +
                       (t2 & 0xff0000) + (t3 & 0xff0000)) >> 2);
        uint32_t g = (((t0 & 0x00ff00) + (t1 & 0x00ff00) +
                       (t2 & 0x00ff00) + (t3 & 0x00ff00)) >> 2);
        uint32_t b = (((t0 & 0x0000ff) + (t1 & 0x0000ff) +
                       (t2 & 0x0000ff) + (t3 & 0x0000ff)) >> 2);
        // diminish
        r = (r * 230) >> 8;
        g = (g * 230) >> 8;
        b = (b * 230) >> 8;
        // store
        dst[x] = (r & 0xff0000) | (g & 0x00ff00) | (b & 0x0000ff);
      }
      dst += size;
    }

  }
}
