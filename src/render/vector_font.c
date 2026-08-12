#include "vector_font.h"

#include <string.h>

#define TL {0.0f, 0.0f}
#define TM {0.5f, 0.0f}
#define TR {1.0f, 0.0f}
#define ML {0.0f, 0.5f}
#define MM {0.5f, 0.5f}
#define MR {1.0f, 0.5f}
#define BL {0.0f, 1.0f}
#define BM {0.5f, 1.0f}
#define BR {1.0f, 1.0f}

static const GlyphSegment GLYPH_0[] = {{TL,TR},{TR,MR},{MR,BR},{BL,BR},{ML,BL},{TL,ML}};
static const GlyphSegment GLYPH_1[] = {{TR,MR},{MR,BR}};
static const GlyphSegment GLYPH_2[] = {{TL,TR},{TR,MR},{ML,MR},{ML,BL},{BL,BR}};
static const GlyphSegment GLYPH_3[] = {{TL,TR},{TR,MR},{ML,MR},{MR,BR},{BL,BR}};
static const GlyphSegment GLYPH_4[] = {{TL,ML},{ML,MR},{TR,MR},{MR,BR}};
static const GlyphSegment GLYPH_5[] = {{TL,TR},{TL,ML},{ML,MR},{MR,BR},{BL,BR}};
static const GlyphSegment GLYPH_6[] = {{TL,TR},{TL,ML},{ML,MR},{ML,BL},{MR,BR},{BL,BR}};
static const GlyphSegment GLYPH_7[] = {{TL,TR},{TR,MR},{MR,BR}};
static const GlyphSegment GLYPH_8[] = {{TL,TR},{TR,MR},{MR,BR},{BL,BR},{ML,BL},{TL,ML},{ML,MR}};
static const GlyphSegment GLYPH_9[] = {{TL,TR},{TR,MR},{MR,BR},{BL,BR},{TL,ML},{ML,MR}};

static const GlyphSegment GLYPH_A[] = {{TL,TR},{TL,ML},{TR,MR},{ML,MR},{ML,BL},{MR,BR}};
static const GlyphSegment GLYPH_C[] = {{TL,TR},{TL,ML},{ML,BL},{BL,BR}};
static const GlyphSegment GLYPH_D[] = {{TL,TR},{TL,ML},{TR,MR},{MR,BR},{BL,BR}};
static const GlyphSegment GLYPH_E[] = {{TL,TR},{TL,ML},{ML,MR},{ML,BL},{BL,BR}};
static const GlyphSegment GLYPH_F[] = {{TL,TR},{TL,BL},{ML,MR}};
static const GlyphSegment GLYPH_G[] = {{TL,TR},{TL,ML},{ML,BL},{BL,BR},{MR,BR},{ML,MR}};
static const GlyphSegment GLYPH_H[] = {{TL,ML},{TR,MR},{ML,MR},{ML,BL},{MR,BR}};
static const GlyphSegment GLYPH_I[] = {{TL,TR},{BL,BR},{TM,MM},{MM,BM}};
static const GlyphSegment GLYPH_L[] = {{TL,ML},{ML,BL},{BL,BR}};
static const GlyphSegment GLYPH_M[] = {{TL,ML},{ML,BL},{TR,MR},{MR,BR},{TL,MM},{TR,MM}};
static const GlyphSegment GLYPH_N[] = {{TL,ML},{ML,BL},{TR,MR},{MR,BR},{TL,BR}};
static const GlyphSegment GLYPH_O[] = {{TL,TR},{TR,MR},{MR,BR},{BL,BR},{ML,BL},{TL,ML}};
static const GlyphSegment GLYPH_P[] = {{TL,TR},{TL,ML},{TR,MR},{ML,MR},{ML,BL}};
static const GlyphSegment GLYPH_R[] = {{TL,TR},{TL,ML},{TR,MR},{ML,MR},{ML,BL},{MM,BR}};
static const GlyphSegment GLYPH_S[] = {{TL,TR},{TL,ML},{ML,MR},{MR,BR},{BL,BR}};
static const GlyphSegment GLYPH_T[] = {{TL,TR},{TM,MM},{MM,BM}};
static const GlyphSegment GLYPH_U[] = {{TL,ML},{ML,BL},{BL,BR},{MR,BR},{TR,MR}};
static const GlyphSegment GLYPH_V[] = {{TL,BM},{TR,BM}};
static const GlyphSegment GLYPH_W[] = {{TL,ML},{ML,BL},{TR,MR},{MR,BR},{BL,MM},{BR,MM}};
static const GlyphSegment GLYPH_X[] = {{TL,BR},{TR,BL}};
static const GlyphSegment GLYPH_COLON[] = {{{0.5f,0.3f},{0.5f,0.35f}},{{0.5f,0.65f},{0.5f,0.7f}}};
static const GlyphSegment GLYPH_PERIOD[] = {{{0.45f,0.95f},{0.55f,1.0f}}};
static const GlyphSegment GLYPH_APOSTROPHE[] = {{TM,{0.5f,0.25f}}};
static const GlyphSegment GLYPH_HYPHEN[] = {{ML,MR}};

typedef struct GlyphEntry {
    char character;
    const GlyphSegment *segments;
    int count;
} GlyphEntry;

#define ENTRY(ch, arr) {ch, arr, (int)(sizeof(arr) / sizeof((arr)[0]))}

static const GlyphEntry GLYPH_TABLE[] = {
    ENTRY('0', GLYPH_0), ENTRY('1', GLYPH_1), ENTRY('2', GLYPH_2), ENTRY('3', GLYPH_3),
    ENTRY('4', GLYPH_4), ENTRY('5', GLYPH_5), ENTRY('6', GLYPH_6), ENTRY('7', GLYPH_7),
    ENTRY('8', GLYPH_8), ENTRY('9', GLYPH_9),
    ENTRY('A', GLYPH_A), ENTRY('C', GLYPH_C), ENTRY('D', GLYPH_D), ENTRY('E', GLYPH_E),
    ENTRY('F', GLYPH_F), ENTRY('G', GLYPH_G), ENTRY('H', GLYPH_H), ENTRY('I', GLYPH_I), ENTRY('L', GLYPH_L),
    ENTRY('M', GLYPH_M), ENTRY('N', GLYPH_N), ENTRY('O', GLYPH_O), ENTRY('P', GLYPH_P),
    ENTRY('R', GLYPH_R), ENTRY('S', GLYPH_S), ENTRY('T', GLYPH_T), ENTRY('U', GLYPH_U),
    ENTRY('V', GLYPH_V), ENTRY('W', GLYPH_W), ENTRY('X', GLYPH_X),
    ENTRY(':', GLYPH_COLON), ENTRY('.', GLYPH_PERIOD), ENTRY('\'', GLYPH_APOSTROPHE),
    ENTRY('-', GLYPH_HYPHEN),
};

#define GLYPH_TABLE_COUNT (int)(sizeof(GLYPH_TABLE) / sizeof(GLYPH_TABLE[0]))

int vector_font_get_glyph(char character, const GlyphSegment **out_segments) {
    if (character == ' ') {
        *out_segments = NULL;
        return 0;
    }
    for (int i = 0; i < GLYPH_TABLE_COUNT; i++) {
        if (GLYPH_TABLE[i].character == character) {
            *out_segments = GLYPH_TABLE[i].segments;
            return GLYPH_TABLE[i].count;
        }
    }
    *out_segments = NULL;
    return 0;
}

float vector_font_text_width(const char *text, float glyph_width, float spacing) {
    size_t length = strlen(text);
    if (length == 0) {
        return 0.0f;
    }
    return (float)length * glyph_width + (float)(length - 1) * spacing;
}
