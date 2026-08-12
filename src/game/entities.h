#ifndef ASTEROIDS_GAME_ENTITIES_H
#define ASTEROIDS_GAME_ENTITIES_H

#include <stdbool.h>

#include "../core/vec2.h"

#define ASTEROID_MAX_VERTICES 16
#define GAME_MAX_ASTEROIDS 96
#define GAME_MAX_BULLETS 32
#define GAME_MAX_PARTICLES 256

typedef enum AsteroidSize {
    ASTEROID_SIZE_LARGE,
    ASTEROID_SIZE_MEDIUM,
    ASTEROID_SIZE_SMALL
} AsteroidSize;

typedef struct Ship {
    Vec2 position;
    Vec2 velocity;
    float rotation;
    bool thrusting;
    bool alive;
    float invulnerable_timer;
    float respawn_timer;
    float fire_cooldown_timer;
} Ship;

typedef struct Bullet {
    Vec2 position;
    Vec2 velocity;
    float lifetime_remaining;
    bool active;
} Bullet;

typedef struct Asteroid {
    Vec2 position;
    Vec2 velocity;
    float rotation;
    float rotation_speed;
    float radius;
    AsteroidSize size;
    bool active;
    Vec2 shape_points[ASTEROID_MAX_VERTICES];
    int vertex_count;
} Asteroid;

typedef struct Particle {
    Vec2 position;
    Vec2 velocity;
    float lifetime_remaining;
    float lifetime_total;
    bool active;
} Particle;

#endif
