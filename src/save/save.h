#ifndef ASTEROIDS_SAVE_SAVE_H
#define ASTEROIDS_SAVE_SAVE_H

#include <stdint.h>
#include <stdbool.h>

#define SAVE_MAGIC 0x44545341u
#define SAVE_VERSION_CURRENT 3u
#define SAVE_MAX_HIGH_SCORES 10
#define SAVE_INITIALS_LENGTH 4

typedef enum SaveControlScheme {
    SAVE_CONTROL_SCHEME_DEFAULT = 0,
    SAVE_CONTROL_SCHEME_ALTERNATE = 1
} SaveControlScheme;

typedef struct SaveHighScoreEntry {
    char initials[SAVE_INITIALS_LENGTH];
    uint32_t score;
} SaveHighScoreEntry;

typedef struct SaveData {
    uint32_t magic;
    uint32_t version;
    SaveHighScoreEntry high_scores[SAVE_MAX_HIGH_SCORES];
    float master_volume;
    float sfx_volume;
    uint32_t fullscreen;
    uint32_t control_scheme;
    uint32_t mouse_control_enabled;
    uint32_t touch_controls_enabled;
    uint32_t checksum;
} SaveData;

void save_set_defaults(SaveData *save);
bool save_load(const char *path, SaveData *save);
bool save_write(const char *path, const SaveData *save);
bool save_try_add_high_score(SaveData *save, uint32_t score, const char *initials);

#endif
