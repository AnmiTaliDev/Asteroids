#include "renderer.h"

#include "vector_font.h"
#include "../core/rng.h"

#include <string.h>

#define GLYPH_SPACING_FACTOR 0.35f

bool renderer_init(Renderer *renderer, const GameConfig *config, const char *title) {
    memset(renderer, 0, sizeof(*renderer));

#ifdef SDL_PLATFORM_ANDROID
    SDL_WindowFlags flags = SDL_WINDOW_FULLSCREEN;
#else
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
    if (config->fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
#endif

    renderer->window = SDL_CreateWindow(title, config->window_width, config->window_height, flags);
    if (renderer->window == NULL) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    renderer->sdl_renderer = SDL_CreateRenderer(renderer->window, NULL);
    if (renderer->sdl_renderer == NULL) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(renderer->window);
        renderer->window = NULL;
        return false;
    }

    SDL_SetRenderVSync(renderer->sdl_renderer, config->vsync ? 1 : 0);
    SDL_SetRenderDrawBlendMode(renderer->sdl_renderer, SDL_BLENDMODE_BLEND);

    renderer->width = config->window_width;
    renderer->height = config->window_height;

    SDL_SetRenderLogicalPresentation(renderer->sdl_renderer, renderer->width, renderer->height,
                                      SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return true;
}

void renderer_shutdown(Renderer *renderer) {
    if (renderer->sdl_renderer != NULL) {
        SDL_DestroyRenderer(renderer->sdl_renderer);
        renderer->sdl_renderer = NULL;
    }
    if (renderer->window != NULL) {
        SDL_DestroyWindow(renderer->window);
        renderer->window = NULL;
    }
}

void renderer_generate_starfield(Renderer *renderer, unsigned int seed) {
    Rng rng;
    rng_seed(&rng, seed);

    renderer->star_count = RENDERER_MAX_STARS;
    for (int i = 0; i < renderer->star_count; i++) {
        renderer->stars[i] = vec2_make(
            rng_range(&rng, 0.0f, (float)renderer->width),
            rng_range(&rng, 0.0f, (float)renderer->height));
        renderer->star_brightness[i] = (Uint8)(rng_range(&rng, 40.0f, 180.0f));
    }
}

void renderer_begin_frame(Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer->sdl_renderer, 4, 4, 12, 255);
    SDL_RenderClear(renderer->sdl_renderer);
}

void renderer_end_frame(Renderer *renderer) {
    SDL_RenderPresent(renderer->sdl_renderer);
}

void renderer_draw_starfield(Renderer *renderer) {
    for (int i = 0; i < renderer->star_count; i++) {
        Uint8 brightness = renderer->star_brightness[i];
        SDL_SetRenderDrawColor(renderer->sdl_renderer, brightness, brightness, brightness, 255);
        SDL_RenderPoint(renderer->sdl_renderer, renderer->stars[i].x, renderer->stars[i].y);
    }
}

void renderer_draw_shape(Renderer *renderer, Vec2 center, float rotation, float scale,
                          const Vec2 *points, int point_count, bool closed, RendererColor color) {
    if (point_count <= 0) {
        return;
    }

    SDL_SetRenderDrawColor(renderer->sdl_renderer, color.r, color.g, color.b, color.a);

    int segment_count = closed ? point_count : point_count - 1;
    for (int i = 0; i < segment_count; i++) {
        Vec2 a = points[i];
        Vec2 b = points[(i + 1) % point_count];

        a = vec2_add(center, vec2_rotate(vec2_scale(a, scale), rotation));
        b = vec2_add(center, vec2_rotate(vec2_scale(b, scale), rotation));

        SDL_RenderLine(renderer->sdl_renderer, a.x, a.y, b.x, b.y);
    }
}

void renderer_draw_point(Renderer *renderer, Vec2 position, RendererColor color) {
    SDL_SetRenderDrawColor(renderer->sdl_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderPoint(renderer->sdl_renderer, position.x, position.y);
}

void renderer_draw_text(Renderer *renderer, Vec2 top_left, float glyph_width, float glyph_height,
                         const char *text, RendererColor color) {
    SDL_SetRenderDrawColor(renderer->sdl_renderer, color.r, color.g, color.b, color.a);

    float spacing = glyph_width * GLYPH_SPACING_FACTOR;
    float cursor_x = top_left.x;

    for (const char *c = text; *c != '\0'; c++) {
        const GlyphSegment *segments = NULL;
        int count = vector_font_get_glyph(*c, &segments);

        for (int i = 0; i < count; i++) {
            Vec2 a = vec2_add(top_left, vec2_make(cursor_x - top_left.x + segments[i].from.x * glyph_width,
                                                    segments[i].from.y * glyph_height));
            Vec2 b = vec2_add(top_left, vec2_make(cursor_x - top_left.x + segments[i].to.x * glyph_width,
                                                    segments[i].to.y * glyph_height));
            SDL_RenderLine(renderer->sdl_renderer, a.x, a.y, b.x, b.y);
        }

        cursor_x += glyph_width + spacing;
    }
}

void renderer_draw_text_centered(Renderer *renderer, Vec2 center, float glyph_width, float glyph_height,
                                  const char *text, RendererColor color) {
    float spacing = glyph_width * GLYPH_SPACING_FACTOR;
    float width = vector_font_text_width(text, glyph_width, spacing);
    Vec2 top_left = vec2_make(center.x - width * 0.5f, center.y - glyph_height * 0.5f);
    renderer_draw_text(renderer, top_left, glyph_width, glyph_height, text, color);
}
