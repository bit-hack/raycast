#pragma once
#include <array>
#include <stdint.h>


struct texture_t {

  texture_t()
    : _loaded(false)
  {}

  bool load(const char *path);

  const uint32_t *getTexture(uint32_t level) const;

  bool loaded() const {
    return _loaded;
  }

protected:

  std::array<uint32_t, 64 * 64 * 2> texel;

  bool _loaded;

  void _genMips(void);
  uint32_t *_getTexture(uint32_t level);
};
