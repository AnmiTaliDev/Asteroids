#ifndef ASTEROIDS_CORE_SHAPES_H
#define ASTEROIDS_CORE_SHAPES_H

#include "vec2.h"

static const Vec2 SHIP_SHAPE_POINTS[] = {
    {1.0f, 0.0f},
    {-0.7f, 0.6f},
    {-0.4f, 0.0f},
    {-0.7f, -0.6f},
};
#define SHIP_SHAPE_POINT_COUNT 4

static const Vec2 SHIP_THRUST_POINTS[] = {
    {-0.4f, 0.25f},
    {-0.9f, 0.0f},
    {-0.4f, -0.25f},
};
#define SHIP_THRUST_POINT_COUNT 3

#endif
