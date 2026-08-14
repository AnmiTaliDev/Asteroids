#ifndef ASTEROIDS_CONFIG_CONFIG_H
#define ASTEROIDS_CONFIG_CONFIG_H

#include <stdbool.h>

typedef struct GameConfig {
    int window_width;
    int window_height;
    bool fullscreen;
    bool vsync;

    float ship_thrust_accel;
    float ship_rotation_speed_deg;
    float ship_max_speed;
    float ship_friction;
    float ship_radius;
    float ship_invulnerability_time;
    float ship_respawn_delay;
    float ship_ram_max_angle_deg;
    float ship_ram_bounce_factor;
    float ship_ram_invulnerability_time;

    float bullet_speed;
    float bullet_lifetime;
    float fire_cooldown;
    int max_bullets_on_screen;

    float asteroid_large_radius;
    float asteroid_medium_radius;
    float asteroid_small_radius;
    float asteroid_min_speed;
    float asteroid_max_speed;
    int asteroid_split_count;
    int asteroid_large_score;
    int asteroid_medium_score;
    int asteroid_small_score;

    int wave_base_asteroid_count;
    int wave_asteroid_increment;
    float wave_start_delay;

    int starting_lives;
    int extra_life_score_interval;

    float master_volume;
    float sfx_volume;

    int starfield_star_count;
} GameConfig;

void config_set_defaults(GameConfig *config);
bool config_load(const char *path, GameConfig *config);
bool config_write_default(const char *path, const GameConfig *config);

#endif
