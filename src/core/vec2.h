#ifndef ASTEROIDS_CORE_VEC2_H
#define ASTEROIDS_CORE_VEC2_H

#include <math.h>

typedef struct Vec2 {
    float x;
    float y;
} Vec2;

static inline Vec2 vec2_make(float x, float y) {
    Vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

static inline Vec2 vec2_add(Vec2 a, Vec2 b) {
    return vec2_make(a.x + b.x, a.y + b.y);
}

static inline Vec2 vec2_sub(Vec2 a, Vec2 b) {
    return vec2_make(a.x - b.x, a.y - b.y);
}

static inline Vec2 vec2_scale(Vec2 a, float s) {
    return vec2_make(a.x * s, a.y * s);
}

static inline float vec2_dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

static inline float vec2_length(Vec2 a) {
    return sqrtf(vec2_dot(a, a));
}

static inline Vec2 vec2_normalize(Vec2 a) {
    float len = vec2_length(a);
    if (len < 1e-6f) {
        return vec2_make(0.0f, 0.0f);
    }
    return vec2_scale(a, 1.0f / len);
}

static inline Vec2 vec2_from_angle(float radians) {
    return vec2_make(cosf(radians), sinf(radians));
}

static inline Vec2 vec2_rotate(Vec2 a, float radians) {
    float c = cosf(radians);
    float s = sinf(radians);
    return vec2_make(a.x * c - a.y * s, a.x * s + a.y * c);
}

#endif
