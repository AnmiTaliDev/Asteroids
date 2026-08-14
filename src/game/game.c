#include "game.h"

#include <string.h>
#include <math.h>

#define ASTEROID_SPAWN_MARGIN 60.0f
#define SHIP_LINE_DRAG_MIN 0.0f

static void push_event(GameContext *ctx, GameEventType type) {
    if (ctx->event_count >= GAME_MAX_EVENTS_PER_FRAME) {
        return;
    }
    ctx->events[ctx->event_count] = type;
    ctx->event_count++;
}

static Vec2 wrap_position(Vec2 position, float width, float height) {
    if (position.x < 0.0f) {
        position.x += width;
    } else if (position.x >= width) {
        position.x -= width;
    }
    if (position.y < 0.0f) {
        position.y += height;
    } else if (position.y >= height) {
        position.y -= height;
    }
    return position;
}

static float asteroid_radius_for_size(const GameConfig *config, AsteroidSize size) {
    switch (size) {
        case ASTEROID_SIZE_LARGE:
            return config->asteroid_large_radius;
        case ASTEROID_SIZE_MEDIUM:
            return config->asteroid_medium_radius;
        case ASTEROID_SIZE_SMALL:
        default:
            return config->asteroid_small_radius;
    }
}

static int asteroid_score_for_size(const GameConfig *config, AsteroidSize size) {
    switch (size) {
        case ASTEROID_SIZE_LARGE:
            return config->asteroid_large_score;
        case ASTEROID_SIZE_MEDIUM:
            return config->asteroid_medium_score;
        case ASTEROID_SIZE_SMALL:
        default:
            return config->asteroid_small_score;
    }
}

static void generate_asteroid_shape(Asteroid *asteroid, Rng *rng) {
    int vertex_count = 8 + (int)(rng_next_u32(rng) % 7u);
    asteroid->vertex_count = vertex_count;
    for (int i = 0; i < vertex_count; i++) {
        float angle = ((float)i / (float)vertex_count) * 6.28318530718f;
        float jitter = rng_range(rng, 0.75f, 1.15f);
        asteroid->shape_points[i] = vec2_make(cosf(angle) * jitter, sinf(angle) * jitter);
    }
}

static Asteroid *find_free_asteroid(GameContext *ctx) {
    for (int i = 0; i < GAME_MAX_ASTEROIDS; i++) {
        if (!ctx->asteroids[i].active) {
            return &ctx->asteroids[i];
        }
    }
    return NULL;
}

static void spawn_asteroid(GameContext *ctx, Vec2 position, AsteroidSize size, Vec2 velocity) {
    Asteroid *asteroid = find_free_asteroid(ctx);
    if (asteroid == NULL) {
        return;
    }

    asteroid->active = true;
    asteroid->position = position;
    asteroid->velocity = velocity;
    asteroid->size = size;
    asteroid->radius = asteroid_radius_for_size(&ctx->config, size);
    asteroid->rotation = rng_range(&ctx->rng, 0.0f, 6.28318530718f);
    asteroid->rotation_speed = rng_range(&ctx->rng, -1.2f, 1.2f);
    generate_asteroid_shape(asteroid, &ctx->rng);
}

static Vec2 random_edge_position(GameContext *ctx) {
    int edge = (int)(rng_next_u32(&ctx->rng) % 4u);
    float x;
    float y;
    switch (edge) {
        case 0:
            x = rng_range(&ctx->rng, 0.0f, ctx->world_width);
            y = -ASTEROID_SPAWN_MARGIN;
            break;
        case 1:
            x = ctx->world_width + ASTEROID_SPAWN_MARGIN;
            y = rng_range(&ctx->rng, 0.0f, ctx->world_height);
            break;
        case 2:
            x = rng_range(&ctx->rng, 0.0f, ctx->world_width);
            y = ctx->world_height + ASTEROID_SPAWN_MARGIN;
            break;
        default:
            x = -ASTEROID_SPAWN_MARGIN;
            y = rng_range(&ctx->rng, 0.0f, ctx->world_height);
            break;
    }
    return vec2_make(x, y);
}

static void spawn_wave(GameContext *ctx) {
    int count = ctx->config.wave_base_asteroid_count + (ctx->wave_number - 1) * ctx->config.wave_asteroid_increment;
    for (int i = 0; i < count; i++) {
        Vec2 position = random_edge_position(ctx);
        Vec2 center = vec2_make(ctx->world_width * 0.5f, ctx->world_height * 0.5f);
        Vec2 direction = vec2_normalize(vec2_sub(center, position));
        float speed = rng_range(&ctx->rng, ctx->config.asteroid_min_speed, ctx->config.asteroid_max_speed);
        Vec2 velocity = vec2_add(vec2_scale(direction, speed), vec2_make(rng_range(&ctx->rng, -20.0f, 20.0f), rng_range(&ctx->rng, -20.0f, 20.0f)));
        spawn_asteroid(ctx, position, ASTEROID_SIZE_LARGE, velocity);
    }
    push_event(ctx, GAME_EVENT_WAVE_STARTED);
}

static void reset_ship(GameContext *ctx) {
    ctx->ship.position = vec2_make(ctx->world_width * 0.5f, ctx->world_height * 0.5f);
    ctx->ship.velocity = vec2_make(0.0f, 0.0f);
    ctx->ship.rotation = -1.57079632679f;
    ctx->ship.thrusting = false;
    ctx->ship.alive = true;
    ctx->ship.invulnerable_timer = ctx->config.ship_invulnerability_time;
    ctx->ship.respawn_timer = 0.0f;
    ctx->ship.fire_cooldown_timer = 0.0f;
}

static void spawn_particle_burst(GameContext *ctx, Vec2 position, int count, float speed_min, float speed_max) {
    for (int i = 0; i < count; i++) {
        Particle *particle = NULL;
        for (int j = 0; j < GAME_MAX_PARTICLES; j++) {
            if (!ctx->particles[j].active) {
                particle = &ctx->particles[j];
                break;
            }
        }
        if (particle == NULL) {
            return;
        }

        float angle = rng_range(&ctx->rng, 0.0f, 6.28318530718f);
        float speed = rng_range(&ctx->rng, speed_min, speed_max);
        particle->active = true;
        particle->position = position;
        particle->velocity = vec2_scale(vec2_from_angle(angle), speed);
        particle->lifetime_total = rng_range(&ctx->rng, 0.4f, 0.9f);
        particle->lifetime_remaining = particle->lifetime_total;
    }
}

void game_init(GameContext *ctx, const GameConfig *config, uint32_t seed, float world_width, float world_height) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->config = *config;
    rng_seed(&ctx->rng, seed);
    ctx->world_width = world_width;
    ctx->world_height = world_height;
    ctx->phase = GAME_PHASE_MENU;
}

void game_start_new_run(GameContext *ctx) {
    for (int i = 0; i < GAME_MAX_ASTEROIDS; i++) {
        ctx->asteroids[i].active = false;
    }
    for (int i = 0; i < GAME_MAX_BULLETS; i++) {
        ctx->bullets[i].active = false;
    }
    for (int i = 0; i < GAME_MAX_PARTICLES; i++) {
        ctx->particles[i].active = false;
    }

    ctx->score = 0;
    ctx->lives = ctx->config.starting_lives;
    ctx->wave_number = 1;
    ctx->next_extra_life_score = (uint32_t)ctx->config.extra_life_score_interval;
    ctx->phase = GAME_PHASE_PLAYING;
    ctx->phase_timer = 0.0f;

    reset_ship(ctx);
    spawn_wave(ctx);
}

int game_asteroids_remaining(const GameContext *ctx) {
    int count = 0;
    for (int i = 0; i < GAME_MAX_ASTEROIDS; i++) {
        if (ctx->asteroids[i].active) {
            count++;
        }
    }
    return count;
}

static void fire_bullet(GameContext *ctx) {
    int active_count = 0;
    for (int i = 0; i < GAME_MAX_BULLETS; i++) {
        if (ctx->bullets[i].active) {
            active_count++;
        }
    }
    if (active_count >= ctx->config.max_bullets_on_screen) {
        return;
    }

    for (int i = 0; i < GAME_MAX_BULLETS; i++) {
        if (!ctx->bullets[i].active) {
            Vec2 direction = vec2_from_angle(ctx->ship.rotation);
            ctx->bullets[i].active = true;
            ctx->bullets[i].position = vec2_add(ctx->ship.position, vec2_scale(direction, ctx->config.ship_radius));
            ctx->bullets[i].velocity = vec2_add(vec2_scale(direction, ctx->config.bullet_speed), ctx->ship.velocity);
            ctx->bullets[i].lifetime_remaining = ctx->config.bullet_lifetime;
            push_event(ctx, GAME_EVENT_BULLET_FIRED);
            return;
        }
    }
}

static void update_ship(GameContext *ctx, float dt, const GameInput *input) {
    Ship *ship = &ctx->ship;

    if (!ship->alive) {
        ship->respawn_timer -= dt;
        if (ship->respawn_timer <= 0.0f && ctx->lives >= 0) {
            reset_ship(ctx);
        }
        return;
    }

    float rotation_speed_rad = ctx->config.ship_rotation_speed_deg * 0.01745329252f;
    if (input->has_target_rotation) {
        float delta = input->target_rotation - ship->rotation;
        while (delta > 3.14159265359f) {
            delta -= 6.28318530718f;
        }
        while (delta < -3.14159265359f) {
            delta += 6.28318530718f;
        }
        float max_step = rotation_speed_rad * dt;
        if (delta > max_step) {
            delta = max_step;
        } else if (delta < -max_step) {
            delta = -max_step;
        }
        ship->rotation += delta;
    } else {
        if (input->rotate_left) {
            ship->rotation -= rotation_speed_rad * dt;
        }
        if (input->rotate_right) {
            ship->rotation += rotation_speed_rad * dt;
        }
    }

    bool was_thrusting = ship->thrusting;
    ship->thrusting = input->thrust;
    if (ship->thrusting && !was_thrusting) {
        push_event(ctx, GAME_EVENT_THRUST_STARTED);
    } else if (!ship->thrusting && was_thrusting) {
        push_event(ctx, GAME_EVENT_THRUST_STOPPED);
    }

    if (ship->thrusting) {
        Vec2 direction = vec2_from_angle(ship->rotation);
        ship->velocity = vec2_add(ship->velocity, vec2_scale(direction, ctx->config.ship_thrust_accel * dt));
    }

    float friction_factor = 1.0f - ctx->config.ship_friction * dt;
    if (friction_factor < SHIP_LINE_DRAG_MIN) {
        friction_factor = SHIP_LINE_DRAG_MIN;
    }
    ship->velocity = vec2_scale(ship->velocity, friction_factor);

    float speed = vec2_length(ship->velocity);
    if (speed > ctx->config.ship_max_speed) {
        ship->velocity = vec2_scale(vec2_normalize(ship->velocity), ctx->config.ship_max_speed);
    }

    ship->position = vec2_add(ship->position, vec2_scale(ship->velocity, dt));
    ship->position = wrap_position(ship->position, ctx->world_width, ctx->world_height);

    if (ship->invulnerable_timer > 0.0f) {
        ship->invulnerable_timer -= dt;
    }

    if (ship->fire_cooldown_timer > 0.0f) {
        ship->fire_cooldown_timer -= dt;
    }

    if (input->fire && ship->fire_cooldown_timer <= 0.0f) {
        fire_bullet(ctx);
        ship->fire_cooldown_timer = ctx->config.fire_cooldown;
    }
}

static void update_bullets(GameContext *ctx, float dt) {
    for (int i = 0; i < GAME_MAX_BULLETS; i++) {
        Bullet *bullet = &ctx->bullets[i];
        if (!bullet->active) {
            continue;
        }
        bullet->lifetime_remaining -= dt;
        if (bullet->lifetime_remaining <= 0.0f) {
            bullet->active = false;
            continue;
        }
        bullet->position = vec2_add(bullet->position, vec2_scale(bullet->velocity, dt));
        bullet->position = wrap_position(bullet->position, ctx->world_width, ctx->world_height);
    }
}

static void update_asteroids(GameContext *ctx, float dt) {
    for (int i = 0; i < GAME_MAX_ASTEROIDS; i++) {
        Asteroid *asteroid = &ctx->asteroids[i];
        if (!asteroid->active) {
            continue;
        }
        asteroid->position = vec2_add(asteroid->position, vec2_scale(asteroid->velocity, dt));
        asteroid->position = wrap_position(asteroid->position, ctx->world_width, ctx->world_height);
        asteroid->rotation += asteroid->rotation_speed * dt;
    }
}

static void update_particles(GameContext *ctx, float dt) {
    for (int i = 0; i < GAME_MAX_PARTICLES; i++) {
        Particle *particle = &ctx->particles[i];
        if (!particle->active) {
            continue;
        }
        particle->lifetime_remaining -= dt;
        if (particle->lifetime_remaining <= 0.0f) {
            particle->active = false;
            continue;
        }
        particle->position = vec2_add(particle->position, vec2_scale(particle->velocity, dt));
        particle->position = wrap_position(particle->position, ctx->world_width, ctx->world_height);
    }
}

static void award_score(GameContext *ctx, int amount) {
    ctx->score += (uint32_t)amount;
    if (ctx->score >= ctx->next_extra_life_score && ctx->config.extra_life_score_interval > 0) {
        ctx->lives++;
        ctx->next_extra_life_score += (uint32_t)ctx->config.extra_life_score_interval;
        push_event(ctx, GAME_EVENT_EXTRA_LIFE);
    }
}

static void destroy_asteroid(GameContext *ctx, Asteroid *asteroid, bool spawn_children) {
    GameEventType event_type;
    AsteroidSize child_size;
    bool has_children;

    switch (asteroid->size) {
        case ASTEROID_SIZE_LARGE:
            event_type = GAME_EVENT_ASTEROID_DESTROYED_LARGE;
            child_size = ASTEROID_SIZE_MEDIUM;
            has_children = spawn_children;
            break;
        case ASTEROID_SIZE_MEDIUM:
            event_type = GAME_EVENT_ASTEROID_DESTROYED_MEDIUM;
            child_size = ASTEROID_SIZE_SMALL;
            has_children = spawn_children;
            break;
        case ASTEROID_SIZE_SMALL:
        default:
            event_type = GAME_EVENT_ASTEROID_DESTROYED_SMALL;
            has_children = false;
            break;
    }

    award_score(ctx, asteroid_score_for_size(&ctx->config, asteroid->size));
    push_event(ctx, event_type);
    spawn_particle_burst(ctx, asteroid->position, 10, 40.0f, 160.0f);

    Vec2 position = asteroid->position;
    Vec2 parent_velocity = asteroid->velocity;
    asteroid->active = false;

    if (has_children) {
        for (int i = 0; i < ctx->config.asteroid_split_count; i++) {
            float angle = rng_range(&ctx->rng, 0.0f, 6.28318530718f);
            float speed = rng_range(&ctx->rng, ctx->config.asteroid_min_speed, ctx->config.asteroid_max_speed);
            Vec2 velocity = vec2_add(parent_velocity, vec2_scale(vec2_from_angle(angle), speed));
            spawn_asteroid(ctx, position, child_size, velocity);
        }
    }
}

static void destroy_ship(GameContext *ctx) {
    push_event(ctx, GAME_EVENT_SHIP_DESTROYED);
    spawn_particle_burst(ctx, ctx->ship.position, 20, 60.0f, 220.0f);
    ctx->ship.alive = false;
    ctx->ship.respawn_timer = ctx->config.ship_respawn_delay;
    ctx->lives--;
}

static void resolve_collisions(GameContext *ctx) {
    for (int a = 0; a < GAME_MAX_ASTEROIDS; a++) {
        Asteroid *asteroid = &ctx->asteroids[a];
        if (!asteroid->active) {
            continue;
        }

        for (int b = 0; b < GAME_MAX_BULLETS; b++) {
            Bullet *bullet = &ctx->bullets[b];
            if (!bullet->active) {
                continue;
            }
            Vec2 diff = vec2_sub(asteroid->position, bullet->position);
            float distance = vec2_length(diff);
            if (distance <= asteroid->radius) {
                bullet->active = false;
                destroy_asteroid(ctx, asteroid, true);
                break;
            }
        }

        if (!asteroid->active) {
            continue;
        }

        if (ctx->ship.alive && ctx->ship.invulnerable_timer <= 0.0f) {
            Vec2 diff = vec2_sub(asteroid->position, ctx->ship.position);
            float distance = vec2_length(diff);
            if (distance <= asteroid->radius + ctx->config.ship_radius) {
                bool rammed = false;
                if (distance > 1e-6f) {
                    Vec2 ship_facing = vec2_from_angle(ctx->ship.rotation);
                    Vec2 to_asteroid = vec2_scale(diff, 1.0f / distance);
                    float alignment = vec2_dot(ship_facing, to_asteroid);
                    float max_angle_rad = ctx->config.ship_ram_max_angle_deg * 0.01745329252f;
                    if (alignment >= cosf(max_angle_rad)) {
                        rammed = true;
                    }
                }

                destroy_asteroid(ctx, asteroid, !rammed);
                if (rammed) {
                    ctx->ship.velocity = vec2_scale(ctx->ship.velocity, -ctx->config.ship_ram_bounce_factor);
                    ctx->ship.invulnerable_timer = ctx->config.ship_ram_invulnerability_time;
                } else {
                    destroy_ship(ctx);
                }
            }
        }
    }
}

void game_update(GameContext *ctx, float dt, const GameInput *input) {
    ctx->event_count = 0;

    switch (ctx->phase) {
        case GAME_PHASE_MENU:
            if (input->start_pressed) {
                game_start_new_run(ctx);
            }
            return;

        case GAME_PHASE_PAUSED:
            if (input->pause_pressed) {
                ctx->phase = GAME_PHASE_PLAYING;
            }
            return;

        case GAME_PHASE_GAME_OVER:
            if (input->start_pressed) {
                ctx->phase = GAME_PHASE_MENU;
            }
            return;

        case GAME_PHASE_WAVE_TRANSITION:
            ctx->phase_timer -= dt;
            update_particles(ctx, dt);
            if (ctx->phase_timer <= 0.0f) {
                ctx->wave_number++;
                spawn_wave(ctx);
                ctx->phase = GAME_PHASE_PLAYING;
            }
            return;

        case GAME_PHASE_PLAYING:
            if (input->pause_pressed) {
                ctx->phase = GAME_PHASE_PAUSED;
                return;
            }
            break;
    }

    update_ship(ctx, dt, input);
    update_bullets(ctx, dt);
    update_asteroids(ctx, dt);
    update_particles(ctx, dt);
    resolve_collisions(ctx);

    if (ctx->lives < 0) {
        ctx->phase = GAME_PHASE_GAME_OVER;
        return;
    }

    if (game_asteroids_remaining(ctx) == 0) {
        ctx->phase = GAME_PHASE_WAVE_TRANSITION;
        ctx->phase_timer = ctx->config.wave_start_delay;
    }
}
