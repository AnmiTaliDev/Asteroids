#ifndef ASTEROIDS_RENDER_RENDERER_H
#define ASTEROIDS_RENDER_RENDERER_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#include "../core/vec2.h"
#include "../config/config.h"

#define RENDERER_MAX_STARS 512

typedef struct RendererColor {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} RendererColor;

typedef struct Renderer {
    SDL_Window *window;
    SDL_Renderer *sdl_renderer;
    int width;
    int height;
    Vec2 stars[RENDERER_MAX_STARS];
    Uint8 star_brightness[RENDERER_MAX_STARS];
    int star_count;
} Renderer;

bool renderer_init(Renderer *renderer, const GameConfig *config, const char *title);
void renderer_shutdown(Renderer *renderer);
void renderer_generate_starfield(Renderer *renderer, unsigned int seed);

void renderer_begin_frame(Renderer *renderer);
void renderer_end_frame(Renderer *renderer);

void renderer_draw_starfield(Renderer *renderer);
void renderer_draw_shape(Renderer *renderer, Vec2 center, float rotation, float scale,
                          const Vec2 *points, int point_count, bool closed, RendererColor color);
void renderer_draw_point(Renderer *renderer, Vec2 position, RendererColor color);
void renderer_draw_text(Renderer *renderer, Vec2 top_left, float glyph_width, float glyph_height,
                         const char *text, RendererColor color);
void renderer_draw_text_centered(Renderer *renderer, Vec2 center, float glyph_width, float glyph_height,
                                  const char *text, RendererColor color);

#endif
