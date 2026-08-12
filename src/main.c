#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "platform/platform.h"
#include "config/config.h"
#include "save/save.h"
#include "game/game.h"
#include "render/renderer.h"
#include "audio/audio.h"
#include "core/shapes.h"

#define GAME_TITLE "Asteroids"
#define MAX_FRAME_DT 0.05f
#define SETTINGS_ITEM_COUNT 5
#define SETTINGS_ITEM_MASTER_VOLUME 0
#define SETTINGS_ITEM_SOUND_VOLUME 1
#define SETTINGS_ITEM_FULLSCREEN 2
#define SETTINGS_ITEM_CONTROLS 3
#define SETTINGS_ITEM_RETURN 4

typedef enum AppState {
    APP_STATE_GAME,
    APP_STATE_SETTINGS
} AppState;

static const RendererColor COLOR_WHITE = {230, 230, 240, 255};
static const RendererColor COLOR_DIM = {130, 130, 150, 255};
static const RendererColor COLOR_SHIP = {200, 240, 255, 255};
static const RendererColor COLOR_ASTEROID = {190, 170, 150, 255};
static const RendererColor COLOR_BULLET = {255, 220, 140, 255};
static const RendererColor COLOR_PARTICLE = {255, 180, 90, 200};
static const RendererColor COLOR_DEBUG = {80, 220, 80, 160};

static void build_input(GameInput *input, const bool *keys, SaveControlScheme control_scheme,
                         bool start_edge, bool pause_edge) {
    SDL_Scancode key_rotate_left = control_scheme == SAVE_CONTROL_SCHEME_ALTERNATE ? SDL_SCANCODE_LEFT : SDL_SCANCODE_A;
    SDL_Scancode key_rotate_right = control_scheme == SAVE_CONTROL_SCHEME_ALTERNATE ? SDL_SCANCODE_RIGHT : SDL_SCANCODE_D;
    SDL_Scancode key_thrust = control_scheme == SAVE_CONTROL_SCHEME_ALTERNATE ? SDL_SCANCODE_UP : SDL_SCANCODE_W;

    memset(input, 0, sizeof(*input));
    input->thrust = keys[key_thrust];
    input->rotate_left = keys[key_rotate_left];
    input->rotate_right = keys[key_rotate_right];
    input->fire = keys[SDL_SCANCODE_SPACE];
    input->start_pressed = start_edge;
    input->pause_pressed = pause_edge;
}

static void draw_ship(Renderer *renderer, const Ship *ship) {
    if (!ship->alive) {
        return;
    }

    bool blinking = ship->invulnerable_timer > 0.0f && fmodf(ship->invulnerable_timer, 0.3f) > 0.15f;
    if (blinking) {
        return;
    }

    renderer_draw_shape(renderer, ship->position, ship->rotation, 22.0f,
                         SHIP_SHAPE_POINTS, SHIP_SHAPE_POINT_COUNT, true, COLOR_SHIP);

    if (ship->thrusting) {
        renderer_draw_shape(renderer, ship->position, ship->rotation, 22.0f,
                             SHIP_THRUST_POINTS, SHIP_THRUST_POINT_COUNT, false, COLOR_BULLET);
    }
}

static void draw_circle_outline(Renderer *renderer, Vec2 center, float radius, RendererColor color) {
    const int segments = 20;
    Vec2 points[20];
    for (int i = 0; i < segments; i++) {
        float angle = ((float)i / (float)segments) * 6.28318530718f;
        points[i] = vec2_make(cosf(angle), sinf(angle));
    }
    renderer_draw_shape(renderer, center, 0.0f, radius, points, segments, true, color);
}

static void draw_hud(Renderer *renderer, const GameContext *ctx) {
    char buffer[64];

    snprintf(buffer, sizeof(buffer), "SCORE %u", ctx->score);
    renderer_draw_text(renderer, vec2_make(20.0f, 16.0f), 14.0f, 20.0f, buffer, COLOR_WHITE);

    snprintf(buffer, sizeof(buffer), "LIVES %d", ctx->lives < 0 ? 0 : ctx->lives);
    renderer_draw_text(renderer, vec2_make(20.0f, 44.0f), 14.0f, 20.0f, buffer, COLOR_DIM);

    snprintf(buffer, sizeof(buffer), "WAVE %d", ctx->wave_number);
    renderer_draw_text(renderer, vec2_make(20.0f, 72.0f), 14.0f, 20.0f, buffer, COLOR_DIM);
}

static void draw_centered_message(Renderer *renderer, float world_width, float world_height,
                                   const char *line1, const char *line2) {
    Vec2 center1 = vec2_make(world_width * 0.5f, world_height * 0.5f - 20.0f);
    Vec2 center2 = vec2_make(world_width * 0.5f, world_height * 0.5f + 24.0f);
    renderer_draw_text_centered(renderer, center1, 20.0f, 28.0f, line1, COLOR_WHITE);
    if (line2 != NULL) {
        renderer_draw_text_centered(renderer, center2, 12.0f, 18.0f, line2, COLOR_DIM);
    }
}

static void draw_settings_screen(Renderer *renderer, float world_width, float world_height,
                                  const SaveData *save, int selected_index) {
    Vec2 center = vec2_make(world_width * 0.5f, world_height * 0.5f - 140.0f);
    renderer_draw_text_centered(renderer, center, 22.0f, 30.0f, "OPTIONS", COLOR_WHITE);

    char lines[SETTINGS_ITEM_COUNT][64];
    snprintf(lines[SETTINGS_ITEM_MASTER_VOLUME], sizeof(lines[0]), "MASTER VOLUME: %d",
             (int)(save->master_volume * 100.0f + 0.5f));
    snprintf(lines[SETTINGS_ITEM_SOUND_VOLUME], sizeof(lines[0]), "SOUND VOLUME: %d",
             (int)(save->sfx_volume * 100.0f + 0.5f));
    snprintf(lines[SETTINGS_ITEM_FULLSCREEN], sizeof(lines[0]), "FULLSCREEN: %s",
             save->fullscreen ? "ON" : "OFF");
    snprintf(lines[SETTINGS_ITEM_CONTROLS], sizeof(lines[0]), "CONTROLS: %s",
             save->control_scheme == SAVE_CONTROL_SCHEME_ALTERNATE ? "ARROWS" : "WASD");
    snprintf(lines[SETTINGS_ITEM_RETURN], sizeof(lines[0]), "RETURN");

    for (int i = 0; i < SETTINGS_ITEM_COUNT; i++) {
        Vec2 row_center = vec2_make(world_width * 0.5f, world_height * 0.5f - 60.0f + (float)i * 40.0f);
        RendererColor color = i == selected_index ? COLOR_WHITE : COLOR_DIM;
        renderer_draw_text_centered(renderer, row_center, 14.0f, 20.0f, lines[i], color);
    }

    Vec2 hint_center = vec2_make(world_width * 0.5f, world_height * 0.5f + 160.0f);
    renderer_draw_text_centered(renderer, hint_center, 10.0f, 14.0f, "ESC TO RETURN", COLOR_DIM);
}

static void adjust_settings_item(int selected_index, int direction, SaveData *save,
                                  AudioSystem *audio, Renderer *renderer) {
    switch (selected_index) {
        case SETTINGS_ITEM_MASTER_VOLUME:
            save->master_volume += (float)direction * 0.05f;
            if (save->master_volume < 0.0f) save->master_volume = 0.0f;
            if (save->master_volume > 1.0f) save->master_volume = 1.0f;
            audio->master_volume = save->master_volume;
            break;
        case SETTINGS_ITEM_SOUND_VOLUME:
            save->sfx_volume += (float)direction * 0.05f;
            if (save->sfx_volume < 0.0f) save->sfx_volume = 0.0f;
            if (save->sfx_volume > 1.0f) save->sfx_volume = 1.0f;
            audio->sfx_volume = save->sfx_volume;
            break;
        case SETTINGS_ITEM_FULLSCREEN:
            save->fullscreen = save->fullscreen ? 0u : 1u;
            SDL_SetWindowFullscreen(renderer->window, save->fullscreen != 0);
            break;
        case SETTINGS_ITEM_CONTROLS:
            save->control_scheme = save->control_scheme == SAVE_CONTROL_SCHEME_ALTERNATE
                                        ? SAVE_CONTROL_SCHEME_DEFAULT
                                        : SAVE_CONTROL_SCHEME_ALTERNATE;
            break;
        default:
            break;
    }
}

static void handle_game_events(GameContext *ctx, AudioSystem *audio) {
    for (int i = 0; i < ctx->event_count; i++) {
        switch (ctx->events[i]) {
            case GAME_EVENT_BULLET_FIRED:
                audio_play(audio, SOUND_FIRE);
                break;
            case GAME_EVENT_THRUST_STARTED:
                audio_set_thrust_playing(audio, true);
                break;
            case GAME_EVENT_THRUST_STOPPED:
                audio_set_thrust_playing(audio, false);
                break;
            case GAME_EVENT_ASTEROID_DESTROYED_LARGE:
                audio_play(audio, SOUND_EXPLOSION_LARGE);
                break;
            case GAME_EVENT_ASTEROID_DESTROYED_MEDIUM:
                audio_play(audio, SOUND_EXPLOSION_MEDIUM);
                break;
            case GAME_EVENT_ASTEROID_DESTROYED_SMALL:
                audio_play(audio, SOUND_EXPLOSION_SMALL);
                break;
            case GAME_EVENT_SHIP_DESTROYED:
                audio_play(audio, SOUND_SHIP_DESTROYED);
                audio_set_thrust_playing(audio, false);
                break;
            case GAME_EVENT_EXTRA_LIFE:
                audio_play(audio, SOUND_EXTRA_LIFE);
                break;
            case GAME_EVENT_WAVE_STARTED:
                break;
        }
    }
}

int main(int argc, char **argv) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Failed to init SDL: %s", SDL_GetError());
        return 1;
    }

    PlatformPaths paths;
    platform_init_paths(&paths, argc > 0 ? argv[0] : NULL);

    GameConfig config;
    config_set_defaults(&config);

    char config_path[PLATFORM_MAX_PATH + 64];
    snprintf(config_path, sizeof(config_path), "%s/game.cfg", paths.config_dir);
    if (!platform_file_exists(config_path)) {
        config_write_default(config_path, &config);
    } else {
        config_load(config_path, &config);
    }

    SaveData save;
    char save_path[PLATFORM_MAX_PATH + 64];
    snprintf(save_path, sizeof(save_path), "%s/savegame.dat", paths.save_dir);
    if (!save_load(save_path, &save)) {
        save_set_defaults(&save);
#ifdef ASTEROIDS_ALTERNATE_CONTROLS
        save.control_scheme = SAVE_CONTROL_SCHEME_ALTERNATE;
#endif
        save_write(save_path, &save);
    }
    config.master_volume = save.master_volume;
    config.sfx_volume = save.sfx_volume;
    config.fullscreen = save.fullscreen != 0;

    Renderer renderer;
    if (!renderer_init(&renderer, &config, GAME_TITLE)) {
        SDL_Quit();
        return 1;
    }
    renderer_generate_starfield(&renderer, 1337);

    AudioSystem audio;
    audio_init(&audio, paths.asset_dir, config.master_volume, config.sfx_volume);

    GameContext game;
    game_init(&game, &config, (uint32_t)SDL_GetTicks(), (float)config.window_width, (float)config.window_height);

    bool running = true;
    Uint64 previous_ticks = SDL_GetTicks();
    bool show_debug_overlay = false;

    AppState app_state = APP_STATE_GAME;
    int settings_selected_index = 0;

    while (running) {
        bool start_edge = false;
        bool pause_edge = false;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                if (app_state == APP_STATE_SETTINGS) {
                    if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                        app_state = APP_STATE_GAME;
                        save_write(save_path, &save);
                    } else if (event.key.scancode == SDL_SCANCODE_UP || event.key.scancode == SDL_SCANCODE_W) {
                        settings_selected_index = (settings_selected_index - 1 + SETTINGS_ITEM_COUNT) % SETTINGS_ITEM_COUNT;
                    } else if (event.key.scancode == SDL_SCANCODE_DOWN || event.key.scancode == SDL_SCANCODE_S) {
                        settings_selected_index = (settings_selected_index + 1) % SETTINGS_ITEM_COUNT;
                    } else if (event.key.scancode == SDL_SCANCODE_LEFT || event.key.scancode == SDL_SCANCODE_A) {
                        adjust_settings_item(settings_selected_index, -1, &save, &audio, &renderer);
                    } else if (event.key.scancode == SDL_SCANCODE_RIGHT || event.key.scancode == SDL_SCANCODE_D) {
                        adjust_settings_item(settings_selected_index, 1, &save, &audio, &renderer);
                    } else if (event.key.scancode == SDL_SCANCODE_RETURN || event.key.scancode == SDL_SCANCODE_SPACE) {
                        if (settings_selected_index == SETTINGS_ITEM_RETURN) {
                            app_state = APP_STATE_GAME;
                            save_write(save_path, &save);
                        } else {
                            adjust_settings_item(settings_selected_index, 1, &save, &audio, &renderer);
                        }
                    }
                } else if (event.key.scancode == SDL_SCANCODE_RETURN) {
                    start_edge = true;
                } else if (event.key.scancode == SDL_SCANCODE_ESCAPE || event.key.scancode == SDL_SCANCODE_P) {
                    pause_edge = true;
                } else if (event.key.scancode == SDL_SCANCODE_O) {
                    if (game.phase == GAME_PHASE_MENU || game.phase == GAME_PHASE_PAUSED) {
                        app_state = APP_STATE_SETTINGS;
                        settings_selected_index = 0;
                    }
                } else if (event.key.scancode == SDL_SCANCODE_F3) {
#if ASTEROIDS_ENABLE_DEBUG_OVERLAY
                    show_debug_overlay = !show_debug_overlay;
#endif
                }
            }
        }

        const bool *keys = SDL_GetKeyboardState(NULL);
        GameInput input;
        build_input(&input, keys, (SaveControlScheme)save.control_scheme, start_edge, pause_edge);

        Uint64 current_ticks = SDL_GetTicks();
        float dt = (float)(current_ticks - previous_ticks) / 1000.0f;
        previous_ticks = current_ticks;
        if (dt > MAX_FRAME_DT) {
            dt = MAX_FRAME_DT;
        }

        uint32_t score_before = game.score;
        GamePhase phase_before = game.phase;

        if (app_state == APP_STATE_GAME) {
            game_update(&game, dt, &input);
            handle_game_events(&game, &audio);
        }
        audio_update(&audio);

        if (phase_before != GAME_PHASE_GAME_OVER && game.phase == GAME_PHASE_GAME_OVER) {
            if (save_try_add_high_score(&save, game.score, "YOU")) {
                save_write(save_path, &save);
            }
        }
        (void)score_before;

        renderer_begin_frame(&renderer);
        renderer_draw_starfield(&renderer);

        if (app_state == APP_STATE_SETTINGS) {
            draw_settings_screen(&renderer, game.world_width, game.world_height, &save, settings_selected_index);
        } else if (game.phase == GAME_PHASE_MENU) {
            draw_centered_message(&renderer, game.world_width, game.world_height,
                                   "ASTEROIDS", "PRESS ENTER TO START");
            char high_score_line[64];
            snprintf(high_score_line, sizeof(high_score_line), "HIGH SCORE %u", save.high_scores[0].score);
            renderer_draw_text_centered(&renderer, vec2_make(game.world_width * 0.5f, game.world_height * 0.5f + 60.0f),
                                         12.0f, 18.0f, high_score_line, COLOR_DIM);
            renderer_draw_text_centered(&renderer, vec2_make(game.world_width * 0.5f, game.world_height * 0.5f + 90.0f),
                                         10.0f, 14.0f, "PRESS O FOR OPTIONS", COLOR_DIM);
        } else {
            for (int i = 0; i < GAME_MAX_ASTEROIDS; i++) {
                const Asteroid *asteroid = &game.asteroids[i];
                if (!asteroid->active) {
                    continue;
                }
                renderer_draw_shape(&renderer, asteroid->position, asteroid->rotation, asteroid->radius,
                                     asteroid->shape_points, asteroid->vertex_count, true, COLOR_ASTEROID);
                if (show_debug_overlay) {
                    draw_circle_outline(&renderer, asteroid->position, asteroid->radius, COLOR_DEBUG);
                }
            }

            for (int i = 0; i < GAME_MAX_BULLETS; i++) {
                const Bullet *bullet = &game.bullets[i];
                if (!bullet->active) {
                    continue;
                }
                renderer_draw_point(&renderer, bullet->position, COLOR_BULLET);
            }

            for (int i = 0; i < GAME_MAX_PARTICLES; i++) {
                const Particle *particle = &game.particles[i];
                if (!particle->active) {
                    continue;
                }
                RendererColor color = COLOR_PARTICLE;
                color.a = (Uint8)(255.0f * (particle->lifetime_remaining / particle->lifetime_total));
                renderer_draw_point(&renderer, particle->position, color);
            }

            draw_ship(&renderer, &game.ship);
            if (show_debug_overlay && game.ship.alive) {
                draw_circle_outline(&renderer, game.ship.position, config.ship_radius, COLOR_DEBUG);
            }

            draw_hud(&renderer, &game);

            if (game.phase == GAME_PHASE_PAUSED) {
                draw_centered_message(&renderer, game.world_width, game.world_height, "PAUSED", "P TO RESUME - O FOR OPTIONS");
            } else if (game.phase == GAME_PHASE_GAME_OVER) {
                draw_centered_message(&renderer, game.world_width, game.world_height, "GAME OVER", "PRESS ENTER");
            } else if (game.phase == GAME_PHASE_WAVE_TRANSITION) {
                char wave_line[32];
                snprintf(wave_line, sizeof(wave_line), "WAVE %d", game.wave_number + 1);
                draw_centered_message(&renderer, game.world_width, game.world_height, wave_line, NULL);
            }
        }

        if (show_debug_overlay) {
            char debug_line[64];
            snprintf(debug_line, sizeof(debug_line), "FPS %d ASTEROIDS %d",
                     dt > 0.0f ? (int)(1.0f / dt) : 0, game_asteroids_remaining(&game));
            renderer_draw_text(&renderer, vec2_make(game.world_width - 320.0f, 16.0f), 10.0f, 14.0f, debug_line, COLOR_DEBUG);
        }

        renderer_end_frame(&renderer);
    }

    save_write(save_path, &save);

    audio_shutdown(&audio);
    renderer_shutdown(&renderer);
    SDL_Quit();
    return 0;
}
