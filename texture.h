#pragma once
#include <array>
#include <stdint.h>

struct texture_t {
  std::array<uint32_t, 64 * 64 * 2> texel;

  bool load(const char *path);

  const uint32_t *getTexture(uint32_t level) const;

protected:
  void _genMips(void);
  uint32_t *_getTexture(uint32_t level);
};
