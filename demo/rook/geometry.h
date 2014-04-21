#ifndef DEMO_ROOK_GEOMETRY_H
#define DEMO_ROOK_GEOMETRY_H

#include <math.h>

namespace demo {
namespace rook {

/*******************************************************************************
 * Vectors!
 */

struct Vec3f;

struct Vec4f {
  float x, y, z, w;

  inline Vec3f project() const;
};

inline constexpr float dot(Vec4f const &a, Vec4f b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

struct Vec3f {
  float x, y, z;

  explicit operator Vec4f() const {
    return { x, y, z, 1 };
  }
};

Vec3f Vec4f::project() const {
  return { x/w, y/w, z/w };
}

struct Vec2f {
  float x, y;
};

struct Vec2i {
  int x, y;
};


/*******************************************************************************
 * Matrices!
 */

struct Mat4f {
  Vec4f r0, r1, r2, r3;

  static constexpr Mat4f identity() {
    return {{ 1, 0, 0, 0 },
            { 0, 1, 0, 0 },
            { 0, 0, 1, 0 },
            { 0, 0, 0, 1 }};
  }

  static constexpr Mat4f translate(float x, float y, float z) {
    return {{ 1, 0, 0, x },
            { 0, 1, 0, y },
            { 0, 0, 1, z },
            { 0, 0, 0, 1 }};
  }

  static constexpr Mat4f scale(float sx, float sy, float sz) {
    return {{ sx, 0,  0,  0 },
            { 0,  sy, 0,  0 },
            { 0,  0,  sz, 0 },
            { 0,  0,  0,  1 }};
  }

  static constexpr Mat4f rotate_z(float a) {
    return {{ cosf(a), -sinf(a), 0, 0 },
            { sinf(a), cosf(a),  0, 0 },
            { 0,      0,         1, 0 },
            { 0,      0,         0, 1 }};
  }

  static constexpr Mat4f rotate_y(float a) {
    return {{ cosf(a),  0, sinf(a), 0 },
            { 0,        1, 0,       0 },
            { -sinf(a), 0, cosf(a), 0 },
            { 0,        0, 0,       1 }};
  }

  inline constexpr Mat4f transpose() const {
    return {{r0.x, r1.x, r2.x, r3.x},
            {r0.y, r1.y, r2.y, r3.y},
            {r0.z, r1.z, r2.z, r3.z},
            {r0.w, r1.w, r2.w, r3.w}};
  }

  inline static constexpr Mat4f ortho(float left, float top,
                                      float right, float bottom,
                                      float near, float far) {
    return {{ 2/(right - left), 0, 0, -(right + left)/(right - left) },
            { 0, 2/(top - bottom), 0, -(top + bottom)/(top - bottom) },
            { 0, 0, -2/(far - near), -(far + near)/(far - near) },
            { 0, 0, 0, 1 }};
  }

  inline static constexpr Mat4f persp(float l, float t,
                                      float r, float b,
                                      float n, float f) {
    return {{ 2*n/(r-l), 0, (r+l)/(r-l), 0 },
            { 0, 2*n/(t-b), (t+b)/(t-b), 0 },
            { 0, 0, -(f+n)/(f-n), -2*f*n/(f-n) },
            { 0, 0, -1, 0 }};
  }
};

__attribute__((section(".ramcode")))
static constexpr Vec4f operator*(Mat4f const &m, Vec4f v) {
  return { dot(m.r0, v),
           dot(m.r1, v),
           dot(m.r2, v),
           dot(m.r3, v) };
}

static constexpr Mat4f operator*(Mat4f const &a, Mat4f const &b) {
  return { b.transpose() * a.r0,
           b.transpose() * a.r1,
           b.transpose() * a.r2,
           b.transpose() * a.r3 };
}

}  // namespace rook
}  // namespace demo

#endif  // DEMO_ROOK_GEOMETRY_H
