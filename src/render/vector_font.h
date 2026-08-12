#ifndef ASTEROIDS_RENDER_VECTOR_FONT_H
#define ASTEROIDS_RENDER_VECTOR_FONT_H

#include "../core/vec2.h"

typedef struct GlyphSegment {
    Vec2 from;
    Vec2 to;
} GlyphSegment;

int vector_font_get_glyph(char character, const GlyphSegment **out_segments);
float vector_font_text_width(const char *text, float glyph_width, float spacing);

#endif
