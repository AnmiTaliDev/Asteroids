#ifndef ASTEROIDS_GAME_GAME_H
#define ASTEROIDS_GAME_GAME_H

#include <stdbool.h>
#include <stdint.h>

#include "entities.h"
#include "../core/rng.h"
#include "../config/config.h"

typedef enum GamePhase {
    GAME_PHASE_MENU,
    GAME_PHASE_PLAYING,
    GAME_PHASE_WAVE_TRANSITION,
    GAME_PHASE_GAME_OVER,
    GAME_PHASE_PAUSED
} GamePhase;

typedef enum GameEventType {
    GAME_EVENT_BULLET_FIRED,
    GAME_EVENT_THRUST_STARTED,
    GAME_EVENT_THRUST_STOPPED,
    GAME_EVENT_ASTEROID_DESTROYED_LARGE,
    GAME_EVENT_ASTEROID_DESTROYED_MEDIUM,
    GAME_EVENT_ASTEROID_DESTROYED_SMALL,
    GAME_EVENT_SHIP_DESTROYED,
    GAME_EVENT_EXTRA_LIFE,
    GAME_EVENT_WAVE_STARTED
} GameEventType;

#define GAME_MAX_EVENTS_PER_FRAME 16

typedef struct GameInput {
    bool thrust;
    bool rotate_left;
    bool rotate_right;
    bool fire;
    bool pause_pressed;
    bool start_pressed;
    bool has_target_rotation;
    float target_rotation;
} GameInput;

typedef struct GameContext {
    GameConfig config;
    Rng rng;

    GamePhase phase;
    float phase_timer;

    Ship ship;
    Asteroid asteroids[GAME_MAX_ASTEROIDS];
    Bullet bullets[GAME_MAX_BULLETS];
    Particle particles[GAME_MAX_PARTICLES];

    uint32_t score;
    int lives;
    int wave_number;
    uint32_t next_extra_life_score;

    float world_width;
    float world_height;

    GameEventType events[GAME_MAX_EVENTS_PER_FRAME];
    int event_count;
} GameContext;

void game_init(GameContext *ctx, const GameConfig *config, uint32_t seed, float world_width, float world_height);
void game_start_new_run(GameContext *ctx);
void game_update(GameContext *ctx, float dt, const GameInput *input);
int game_asteroids_remaining(const GameContext *ctx);

#endif
