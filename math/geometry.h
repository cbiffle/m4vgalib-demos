#ifndef MATH_GEOMETRY_H
#define MATH_GEOMETRY_H

#include <cmath>

namespace math {

/*******************************************************************************
 * Vectors!
 */

struct Vec3f;

struct Vec4f {
  float x, y, z, w;

  inline constexpr Vec3f project() const;
};

inline constexpr float dot(Vec4f const &a, Vec4f b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline constexpr Vec4f operator-(Vec4f const &a, Vec4f const &b) {
  return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

inline constexpr Vec4f operator+(Vec4f const &a, Vec4f const &b) {
  return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

inline constexpr Vec4f operator*(Vec4f const &a, float b) {
  return { a.x * b, a.y * b, a.z * b, a.w * b };
}

inline constexpr Vec4f operator*(float b, Vec4f const &a) {
  return { a.x * b, a.y * b, a.z * b, a.w * b };
}


struct Vec2f;
struct Vec3h;

struct Vec3f {
  float x, y, z;

  explicit operator Vec4f() const {
    return { x, y, z, 1 };
  }

  inline constexpr Vec2f xy() const;
  inline constexpr Vec2f hom() const;

  inline explicit operator Vec3h() const;

  constexpr float magnitude() const {
    return std::sqrt(x * x + y * y + z * z);
  }

  inline constexpr Vec3f cross(Vec3f other) {
    return {
      y * other.z - z * other.y,
      z * other.x - x * other.z,
      x * other.y - y * other.x,
    };
  }

  constexpr Vec3f normalized() const {
    return { x / magnitude(), y / magnitude(), z / magnitude() };
  }
};

constexpr Vec3f Vec4f::project() const {
  return { x/w, y/w, z/w };
}

inline constexpr float dot(Vec3f const &a, Vec3f b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline constexpr Vec3f operator-(Vec3f const &a, Vec3f const &b) {
  return { a.x - b.x, a.y - b.y, a.z - b.z };
}

inline constexpr Vec3f operator+(Vec3f const &a, Vec3f const &b) {
  return { a.x + b.x, a.y + b.y, a.z + b.z };
}

inline constexpr Vec3f operator*(Vec3f const &a, float b) {
  return { a.x * b, a.y * b, a.z * b };
}

inline constexpr Vec3f operator*(float b, Vec3f const &a) {
  return { a.x * b, a.y * b, a.z * b };
}


struct Vec2f {
  float x, y;

  constexpr float magnitude() const {
    return std::sqrt(x * x + y * y);
  }

  constexpr Vec2f normalized() const {
    return { x / magnitude(), y / magnitude() };
  }
};

constexpr Vec2f Vec3f::xy() const {
  return { x, y };
}

constexpr Vec2f Vec3f::hom() const {
  return { x/z, y/z };
}

inline constexpr float dot(Vec2f const &a, Vec2f b) {
  return a.x * b.x + a.y * b.y;
}

inline constexpr Vec2f operator-(Vec2f const &a, Vec2f const &b) {
  return { a.x - b.x, a.y - b.y };
}

inline constexpr Vec2f operator+(Vec2f const &a, Vec2f const &b) {
  return { a.x + b.x, a.y + b.y };
}

inline constexpr Vec2f operator*(Vec2f const &a, float b) {
  return { a.x * b, a.y * b };
}

inline constexpr Vec2f operator*(float b, Vec2f const &a) {
  return { a.x * b, a.y * b };
}

struct Vec2i {
  int x, y;
};

struct Vec3h {
  __fp16 x, y, z;

  explicit operator Vec4f() const {
    return { float(x), float(y), float(z), 1 };
  }
};

inline Vec3f::operator Vec3h() const {
  return { __fp16(x), __fp16(y), __fp16(z) };
}


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

  static constexpr Mat4f rotate_x(float a) {
    return {{ 1, 0,       0,        0 },
            { 0, cosf(a), -sinf(a), 0 },
            { 0, sinf(a), cosf(a),  0 },
            { 0, 0,       0,        1 }};
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

struct Mat3f {
  Vec3f r0, r1, r2;

  static constexpr Mat3f identity() {
    return {{ 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 }};
  }

  static constexpr Mat3f translate(float x, float y) {
    return {{ 1, 0, x },
            { 0, 1, y },
            { 0, 0, 1 }};
  }

  static constexpr Mat3f scale(float sx, float sy) {
    return {{ sx, 0,  0 },
            { 0,  sy, 0 },
            { 0,  0,  1 }};
  }

  static constexpr Mat3f rotate(float a) {
    return {{ cosf(a), -sinf(a), 0 },
            { sinf(a), cosf(a),  0 },
            { 0,      0,         1 }};
  }

  inline constexpr Mat3f transpose() const {
    return {{r0.x, r1.x, r2.x},
            {r0.y, r1.y, r2.y},
            {r0.z, r1.z, r2.z}};
  }
};

__attribute__((section(".ramcode")))
static constexpr Vec3f operator*(Mat3f const &m, Vec3f v) {
  return { dot(m.r0, v),
           dot(m.r1, v),
           dot(m.r2, v) };
}

static constexpr Mat3f operator*(Mat3f const &a, Mat3f const &b) {
  return { b.transpose() * a.r0,
           b.transpose() * a.r1,
           b.transpose() * a.r2 };
}

struct Mat2f {
  Vec2f r0, r1;

  static constexpr Mat2f identity() {
    return {{ 1, 0 },
            { 0, 1 }};
  }

  static constexpr Mat2f rotate(float a) {
    return {{ cosf(a), -sinf(a) },
            { sinf(a), cosf(a)  }};
  }

  inline constexpr Mat2f transpose() const {
    return {{r0.x, r1.x},
            {r0.y, r1.y}};
  }
};

__attribute__((section(".ramcode")))
static constexpr Vec2f operator*(Mat2f const &m, Vec2f v) {
  return { dot(m.r0, v),
           dot(m.r1, v) };
}

static constexpr Mat2f operator*(Mat2f const &a, Mat2f const &b) {
  return { b.transpose() * a.r0,
           b.transpose() * a.r1 };
}

}  // namespace math

#endif  // MATH_GEOMETRY_H
