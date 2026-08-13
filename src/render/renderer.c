#include "renderer.h"

#include "../core/rng.h"

#include <string.h>
#include <stdio.h>

#define LINE_JOINT_OVERLAP 0.5f
#define FONT_FILE_NAME "PressStart2P-Regular.ttf"

static void draw_line_with_overlap(SDL_Renderer *sdl_renderer, Vec2 a, Vec2 b) {
    Vec2 direction = vec2_sub(b, a);
    float length = vec2_length(direction);
    if (length > 1e-6f) {
        Vec2 extend = vec2_scale(direction, LINE_JOINT_OVERLAP / length);
        a = vec2_sub(a, extend);
        b = vec2_add(b, extend);
    }
    SDL_RenderLine(sdl_renderer, a.x, a.y, b.x, b.y);
}

bool renderer_init(Renderer *renderer, const GameConfig *config, const char *title, const char *asset_dir) {
    memset(renderer, 0, sizeof(*renderer));

    if (!TTF_Init()) {
        SDL_Log("Failed to init SDL_ttf: %s", SDL_GetError());
        return false;
    }

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

    char font_path[1024];
    if (asset_dir[0] == '\0') {
        snprintf(font_path, sizeof(font_path), "fonts/%s", FONT_FILE_NAME);
    } else {
        snprintf(font_path, sizeof(font_path), "%s/fonts/%s", asset_dir, FONT_FILE_NAME);
    }
    renderer->font = TTF_OpenFont(font_path, 24.0f);
    if (renderer->font == NULL) {
        SDL_Log("Failed to load font '%s': %s", font_path, SDL_GetError());
        SDL_DestroyRenderer(renderer->sdl_renderer);
        SDL_DestroyWindow(renderer->window);
        renderer->sdl_renderer = NULL;
        renderer->window = NULL;
        return false;
    }

    renderer->width = config->window_width;
    renderer->height = config->window_height;

    SDL_SetRenderLogicalPresentation(renderer->sdl_renderer, renderer->width, renderer->height,
                                      SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return true;
}

void renderer_shutdown(Renderer *renderer) {
    if (renderer->font != NULL) {
        TTF_CloseFont(renderer->font);
        renderer->font = NULL;
    }
    if (renderer->sdl_renderer != NULL) {
        SDL_DestroyRenderer(renderer->sdl_renderer);
        renderer->sdl_renderer = NULL;
    }
    if (renderer->window != NULL) {
        SDL_DestroyWindow(renderer->window);
        renderer->window = NULL;
    }
    TTF_Quit();
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

        draw_line_with_overlap(renderer->sdl_renderer, a, b);
    }
}

void renderer_draw_point(Renderer *renderer, Vec2 position, RendererColor color) {
    SDL_SetRenderDrawColor(renderer->sdl_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderPoint(renderer->sdl_renderer, position.x, position.y);
}

void renderer_draw_circle_outline(Renderer *renderer, Vec2 center, float radius, RendererColor color) {
    const int segments = 20;
    Vec2 points[20];
    for (int i = 0; i < segments; i++) {
        float angle = ((float)i / (float)segments) * 6.28318530718f;
        points[i] = vec2_make(cosf(angle), sinf(angle));
    }
    renderer_draw_shape(renderer, center, 0.0f, radius, points, segments, true, color);
}

void renderer_draw_rect_outline(Renderer *renderer, Vec2 center, float half_width, float half_height,
                                 RendererColor color) {
    Vec2 points[4] = {
        {-half_width, -half_height},
        {half_width, -half_height},
        {half_width, half_height},
        {-half_width, half_height},
    };
    renderer_draw_shape(renderer, center, 0.0f, 1.0f, points, 4, true, color);
}

void renderer_draw_text(Renderer *renderer, Vec2 top_left, float pixel_height,
                         const char *text, RendererColor color) {
    if (text[0] == '\0') {
        return;
    }

    TTF_SetFontSize(renderer->font, pixel_height);

    SDL_Color sdl_color = {color.r, color.g, color.b, color.a};
    SDL_Surface *surface = TTF_RenderText_Blended(renderer->font, text, 0, sdl_color);
    if (surface == NULL) {
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer->sdl_renderer, surface);
    if (texture != NULL) {
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        SDL_FRect destination = {top_left.x, top_left.y, (float)surface->w, (float)surface->h};
        SDL_RenderTexture(renderer->sdl_renderer, texture, NULL, &destination);
        SDL_DestroyTexture(texture);
    }

    SDL_DestroySurface(surface);
}

void renderer_draw_text_centered(Renderer *renderer, Vec2 center, float pixel_height,
                                  const char *text, RendererColor color) {
    if (text[0] == '\0') {
        return;
    }

    TTF_SetFontSize(renderer->font, pixel_height);

    int width = 0;
    int height = 0;
    TTF_GetStringSize(renderer->font, text, 0, &width, &height);

    Vec2 top_left = vec2_make(center.x - (float)width * 0.5f, center.y - (float)height * 0.5f);
    renderer_draw_text(renderer, top_left, pixel_height, text, color);
}
