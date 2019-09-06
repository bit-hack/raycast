#include <array>
#include <stdint.h>

#ifndef _MSC_VER
#include <x86intrin.h>
#else
#include <xmmintrin.h>
#endif

#include "common.h"


// XXX: align me!
// XXX: round up width
alignas(16) std::array<uint32_t, screen_w*screen_h> screen;
alignas(16) std::array<uint16_t, screen_w*screen_h> depth;

void present_screen(SDL_Surface *surf) {

  // XXX: do lighting in here!

  const uint32_t *src = screen.data();
  const uint16_t *dth = depth.data();
  uint32_t *dst = (uint32_t*)surf->pixels;
  const uint32_t pitch = surf->pitch / 4;
  for (uint32_t y = 0; y < screen_h; ++y) {
    for (uint32_t x = 0; x < screen_w; ++x) {

      const uint8_t d = 0xff - (dth[x] >> 5);

      const uint32_t rgb = src[x];

      const uint8_t r = ((((rgb >> 16) & 0xff) * d) >> 8) & 0xff;
      const uint8_t g = ((((rgb >>  8) & 0xff) * d) >> 8) & 0xff;
      const uint8_t b = ((((rgb >>  0) & 0xff) * d) >> 8) & 0xff;

      const uint32_t clr = (r << 16) | (g << 8) | b;

      dst[x * 2 + 0] = clr;
      dst[x * 2 + 1] = clr;
      dst[x * 2 + 0 + pitch] = clr;
      dst[x * 2 + 1 + pitch] = clr;
    }
    src += screen_w;
    dth += screen_w;
    dst += pitch * 2;
  }
}

static inline __m128i _mul(__m128i a, __m128i b) {
  return _mm_mullo_epi32(a, b);
}

void present_screen_sse(SDL_Surface *surf) {

  const uint32_t *src = screen.data();
  const uint16_t *dth = depth.data();
  uint32_t *dst = (uint32_t*)surf->pixels;
  const uint32_t pitch = surf->pitch / 4;
  for (uint32_t y = 0; y < screen_h; ++y) {

    uint32_t *dx0 = dst;
    uint32_t *dx1 = dst + pitch;

    for (uint32_t x = 0; x < screen_w; x += 4) {
      __m128i m = _mm_set_epi32(0xff, 0xff, 0xff, 0xff);
      // XXX: do lighting here!
      // load depth map
      __m128i d   = _mm_loadu_si128((const __m128i*)(dth + x));
      __m128i dt0 = _mm_srli_epi16(d, 5);
      __m128i dt1 = _mm_cvtepu16_epi32(dt0);
      __m128i dt2 = _mm_sub_epi32(m, dt1);
      // load pixels
      __m128i c = _mm_loadu_si128((const __m128i*)(src + x));
      // red
      __m128i r   = _mm_srli_epi32(c, 16);
      __m128i rt0 = _mm_and_si128(r, m);
      __m128i rt1 = _mul(rt0, dt2);
      __m128i rt2 = _mm_slli_epi32(rt1, 8);
      __m128i rt3 = _mm_and_si128(rt2,
                      _mm_set_epi32(0xff0000, 0xff0000, 0xff0000, 0xff0000));
      // green
      __m128i g   = _mm_srli_epi32(c, 8);
      __m128i gt0 = _mm_and_si128(g, m);
      __m128i gt1 = _mul(gt0, dt2);
      __m128i gt2 = _mm_and_si128(gt1,
                      _mm_set_epi32(0xff00, 0xff00, 0xff00, 0xff00));
      // blue
      __m128i b   = _mm_srli_epi32(c, 0);
      __m128i bt0 = _mm_and_si128(b, m);
      __m128i bt1 = _mul(bt0, dt2);
      __m128i bt2 = _mm_srli_epi32(bt1, 8);
      // blend all channels back together
      __m128i f =  _mm_or_si128(rt3, _mm_or_si128(gt2, bt2));
      // scale pixels x2 horizontaly
      __m128i p0 = _mm_unpacklo_epi32(f, f);
      __m128i p1 = _mm_unpackhi_epi32(f, f);
      // store to screenbuffer
      _mm_storeu_si128((__m128i*)(dx0 + x * 2 + 0), p0);
      _mm_storeu_si128((__m128i*)(dx1 + x * 2 + 0), p0);
      _mm_storeu_si128((__m128i*)(dx0 + x * 2 + 4), p1);
      _mm_storeu_si128((__m128i*)(dx1 + x * 2 + 4), p1);
    }

    src += screen_w;
    dth += screen_w;
    dst += pitch * 2;
  }
}

void present_depth(SDL_Surface *surf) {
  const uint16_t *src = depth.data();
  uint32_t *dst = (uint32_t*)surf->pixels;
  const uint32_t pitch = surf->pitch / 4;
  for (uint32_t y = 0; y < screen_h; ++y) {
    for (uint32_t x = 0; x < screen_w; ++x) {
      const uint32_t rgb = (src[x] * 8) & 0xff00;
      dst[x * 2 + 0] = rgb;
      dst[x * 2 + 1] = rgb;
      dst[x * 2 + 0 + pitch] = rgb;
      dst[x * 2 + 1 + pitch] = rgb;
    }
    src += screen_w;
    dst += pitch * 2;
  }
}
