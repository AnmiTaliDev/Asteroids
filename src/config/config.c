#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void config_set_defaults(GameConfig *config) {
    config->window_width = 1280;
    config->window_height = 720;
    config->fullscreen = false;
    config->vsync = true;

    config->ship_thrust_accel = 260.0f;
    config->ship_rotation_speed_deg = 220.0f;
    config->ship_max_speed = 420.0f;
    config->ship_friction = 0.35f;
    config->ship_radius = 14.0f;
    config->ship_invulnerability_time = 2.5f;
    config->ship_respawn_delay = 1.5f;
    config->ship_ram_max_angle_deg = 40.0f;
    config->ship_ram_bounce_factor = 0.4f;
    config->ship_ram_invulnerability_time = 0.6f;

    config->bullet_speed = 620.0f;
    config->bullet_lifetime = 0.9f;
    config->fire_cooldown = 0.22f;
    config->max_bullets_on_screen = 6;

    config->asteroid_large_radius = 48.0f;
    config->asteroid_medium_radius = 28.0f;
    config->asteroid_small_radius = 14.0f;
    config->asteroid_min_speed = 40.0f;
    config->asteroid_max_speed = 110.0f;
    config->asteroid_split_count = 2;
    config->asteroid_large_score = 20;
    config->asteroid_medium_score = 50;
    config->asteroid_small_score = 100;

    config->wave_base_asteroid_count = 4;
    config->wave_asteroid_increment = 2;
    config->wave_start_delay = 2.0f;

    config->starting_lives = 3;
    config->extra_life_score_interval = 10000;

    config->master_volume = 0.8f;
    config->sfx_volume = 1.0f;

    config->starfield_star_count = 160;
}

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) {
        s++;
    }
    if (*s == '\0') {
        return s;
    }
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    return s;
}

static bool parse_bool(const char *value) {
    return strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "yes") == 0;
}

static void apply_key_value(GameConfig *config, const char *key, const char *value) {
    if (strcmp(key, "window_width") == 0) {
        config->window_width = atoi(value);
    } else if (strcmp(key, "window_height") == 0) {
        config->window_height = atoi(value);
    } else if (strcmp(key, "fullscreen") == 0) {
        config->fullscreen = parse_bool(value);
    } else if (strcmp(key, "vsync") == 0) {
        config->vsync = parse_bool(value);
    } else if (strcmp(key, "ship_thrust_accel") == 0) {
        config->ship_thrust_accel = strtof(value, NULL);
    } else if (strcmp(key, "ship_rotation_speed_deg") == 0) {
        config->ship_rotation_speed_deg = strtof(value, NULL);
    } else if (strcmp(key, "ship_max_speed") == 0) {
        config->ship_max_speed = strtof(value, NULL);
    } else if (strcmp(key, "ship_friction") == 0) {
        config->ship_friction = strtof(value, NULL);
    } else if (strcmp(key, "ship_radius") == 0) {
        config->ship_radius = strtof(value, NULL);
    } else if (strcmp(key, "ship_invulnerability_time") == 0) {
        config->ship_invulnerability_time = strtof(value, NULL);
    } else if (strcmp(key, "ship_respawn_delay") == 0) {
        config->ship_respawn_delay = strtof(value, NULL);
    } else if (strcmp(key, "ship_ram_max_angle_deg") == 0) {
        config->ship_ram_max_angle_deg = strtof(value, NULL);
    } else if (strcmp(key, "ship_ram_bounce_factor") == 0) {
        config->ship_ram_bounce_factor = strtof(value, NULL);
    } else if (strcmp(key, "ship_ram_invulnerability_time") == 0) {
        config->ship_ram_invulnerability_time = strtof(value, NULL);
    } else if (strcmp(key, "bullet_speed") == 0) {
        config->bullet_speed = strtof(value, NULL);
    } else if (strcmp(key, "bullet_lifetime") == 0) {
        config->bullet_lifetime = strtof(value, NULL);
    } else if (strcmp(key, "fire_cooldown") == 0) {
        config->fire_cooldown = strtof(value, NULL);
    } else if (strcmp(key, "max_bullets_on_screen") == 0) {
        config->max_bullets_on_screen = atoi(value);
    } else if (strcmp(key, "asteroid_large_radius") == 0) {
        config->asteroid_large_radius = strtof(value, NULL);
    } else if (strcmp(key, "asteroid_medium_radius") == 0) {
        config->asteroid_medium_radius = strtof(value, NULL);
    } else if (strcmp(key, "asteroid_small_radius") == 0) {
        config->asteroid_small_radius = strtof(value, NULL);
    } else if (strcmp(key, "asteroid_min_speed") == 0) {
        config->asteroid_min_speed = strtof(value, NULL);
    } else if (strcmp(key, "asteroid_max_speed") == 0) {
        config->asteroid_max_speed = strtof(value, NULL);
    } else if (strcmp(key, "asteroid_split_count") == 0) {
        config->asteroid_split_count = atoi(value);
    } else if (strcmp(key, "asteroid_large_score") == 0) {
        config->asteroid_large_score = atoi(value);
    } else if (strcmp(key, "asteroid_medium_score") == 0) {
        config->asteroid_medium_score = atoi(value);
    } else if (strcmp(key, "asteroid_small_score") == 0) {
        config->asteroid_small_score = atoi(value);
    } else if (strcmp(key, "wave_base_asteroid_count") == 0) {
        config->wave_base_asteroid_count = atoi(value);
    } else if (strcmp(key, "wave_asteroid_increment") == 0) {
        config->wave_asteroid_increment = atoi(value);
    } else if (strcmp(key, "wave_start_delay") == 0) {
        config->wave_start_delay = strtof(value, NULL);
    } else if (strcmp(key, "starting_lives") == 0) {
        config->starting_lives = atoi(value);
    } else if (strcmp(key, "extra_life_score_interval") == 0) {
        config->extra_life_score_interval = atoi(value);
    } else if (strcmp(key, "master_volume") == 0) {
        config->master_volume = strtof(value, NULL);
    } else if (strcmp(key, "sfx_volume") == 0) {
        config->sfx_volume = strtof(value, NULL);
    } else if (strcmp(key, "starfield_star_count") == 0) {
        config->starfield_star_count = atoi(value);
    } else {
        fprintf(stderr, "config: ignoring unknown key '%s'\n", key);
    }
}

bool config_load(const char *path, GameConfig *config) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        char *trimmed = trim(line);
        if (trimmed[0] == '\0' || trimmed[0] == '#') {
            continue;
        }

        char *equals = strchr(trimmed, '=');
        if (equals == NULL) {
            continue;
        }

        *equals = '\0';
        char *key = trim(trimmed);
        char *value = trim(equals + 1);
        apply_key_value(config, key, value);
    }

    fclose(file);
    return true;
}

bool config_write_default(const char *path, const GameConfig *config) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        return false;
    }

    fprintf(file,
        "# Asteroids game configuration\n"
        "# Lines starting with # are comments. Format: key = value\n"
        "# Delete this file and relaunch the game to regenerate these defaults.\n\n");

    fprintf(file, "# Display\n");
    fprintf(file, "window_width = %d\n", config->window_width);
    fprintf(file, "window_height = %d\n", config->window_height);
    fprintf(file, "fullscreen = %s\n", config->fullscreen ? "true" : "false");
    fprintf(file, "vsync = %s\n\n", config->vsync ? "true" : "false");

    fprintf(file, "# Ship handling\n");
    fprintf(file, "ship_thrust_accel = %.2f\n", (double)config->ship_thrust_accel);
    fprintf(file, "ship_rotation_speed_deg = %.2f\n", (double)config->ship_rotation_speed_deg);
    fprintf(file, "ship_max_speed = %.2f\n", (double)config->ship_max_speed);
    fprintf(file, "ship_friction = %.3f\n", (double)config->ship_friction);
    fprintf(file, "ship_radius = %.2f\n", (double)config->ship_radius);
    fprintf(file, "ship_invulnerability_time = %.2f\n", (double)config->ship_invulnerability_time);
    fprintf(file, "ship_respawn_delay = %.2f\n", (double)config->ship_respawn_delay);
    fprintf(file, "ship_ram_max_angle_deg = %.2f\n", (double)config->ship_ram_max_angle_deg);
    fprintf(file, "ship_ram_bounce_factor = %.3f\n", (double)config->ship_ram_bounce_factor);
    fprintf(file, "ship_ram_invulnerability_time = %.2f\n\n", (double)config->ship_ram_invulnerability_time);

    fprintf(file, "# Weapon\n");
    fprintf(file, "bullet_speed = %.2f\n", (double)config->bullet_speed);
    fprintf(file, "bullet_lifetime = %.2f\n", (double)config->bullet_lifetime);
    fprintf(file, "fire_cooldown = %.3f\n", (double)config->fire_cooldown);
    fprintf(file, "max_bullets_on_screen = %d\n\n", config->max_bullets_on_screen);

    fprintf(file, "# Asteroids\n");
    fprintf(file, "asteroid_large_radius = %.2f\n", (double)config->asteroid_large_radius);
    fprintf(file, "asteroid_medium_radius = %.2f\n", (double)config->asteroid_medium_radius);
    fprintf(file, "asteroid_small_radius = %.2f\n", (double)config->asteroid_small_radius);
    fprintf(file, "asteroid_min_speed = %.2f\n", (double)config->asteroid_min_speed);
    fprintf(file, "asteroid_max_speed = %.2f\n", (double)config->asteroid_max_speed);
    fprintf(file, "asteroid_split_count = %d\n", config->asteroid_split_count);
    fprintf(file, "asteroid_large_score = %d\n", config->asteroid_large_score);
    fprintf(file, "asteroid_medium_score = %d\n", config->asteroid_medium_score);
    fprintf(file, "asteroid_small_score = %d\n\n", config->asteroid_small_score);

    fprintf(file, "# Waves and difficulty\n");
    fprintf(file, "wave_base_asteroid_count = %d\n", config->wave_base_asteroid_count);
    fprintf(file, "wave_asteroid_increment = %d\n", config->wave_asteroid_increment);
    fprintf(file, "wave_start_delay = %.2f\n\n", (double)config->wave_start_delay);

    fprintf(file, "# Lives and scoring\n");
    fprintf(file, "starting_lives = %d\n", config->starting_lives);
    fprintf(file, "extra_life_score_interval = %d\n\n", config->extra_life_score_interval);

    fprintf(file, "# Audio\n");
    fprintf(file, "master_volume = %.2f\n", (double)config->master_volume);
    fprintf(file, "sfx_volume = %.2f\n\n", (double)config->sfx_volume);

    fprintf(file, "# Visuals\n");
    fprintf(file, "starfield_star_count = %d\n", config->starfield_star_count);

    fclose(file);
    return true;
}
