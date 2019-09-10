#pragma once

template <typename type_t>
struct vec2_t {
  type_t x, y;

  vec2_t operator + (const vec2_t &a) const {
    return vec2_t{a.x + x, a.y + y};
  }

  void operator += (const vec2_t &a) {
    x += a.x; y += a.y;
  }

  static type_t dot(const vec2_t &a, const vec2_t &b) {
    return a.x * b.x + a.y * b.y;
  }
};

template <typename type_t>
vec2_t<type_t> operator - (const vec2_t<type_t> &a, const vec2_t<type_t> &b) {
  return vec2_t<type_t>{a.x - b.x, a.y - b.y};
}

template <typename type_t>
vec2_t<type_t> operator / (const vec2_t<type_t> &a, const type_t b) {
  return vec2_t<type_t>{a.x / b, a.y / b};
}

template <typename type_t>
vec2_t<type_t> operator * (const vec2_t<type_t> &a, const type_t b) {
  return vec2_t<type_t>{a.x * b, a.y * b};
}

template <typename type_t>
struct vec3_t {
  type_t x, y, z;

  vec3_t operator + (const vec3_t &a) const {
    return vec3_t{a.x + x, a.y + y, a.z + z};
  }

  void operator += (const vec3_t &a) {
    x += a.x; y += a.y; z += a.z;
  }

  void operator *= (const float &a) {
    x *= a; y *= a; z *= a;
  }

  static type_t dot(const vec3_t &a, const vec3_t &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  }
};

using vec2f_t = vec2_t<float>;
using vec2i_t = vec2_t<int32_t>;

using vec3f_t = vec3_t<float>;
using vec3i_t = vec3_t<int32_t>;
