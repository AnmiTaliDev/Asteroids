#include "save.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>

static uint32_t crc32_compute(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            uint32_t mask = (uint32_t)(-(int32_t)(crc & 1u));
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}

static uint32_t save_checksum(const SaveData *save) {
    size_t checksum_offset = offsetof(SaveData, checksum);
    return crc32_compute((const uint8_t *)save, checksum_offset);
}

void save_set_defaults(SaveData *save) {
    memset(save, 0, sizeof(*save));
    save->magic = SAVE_MAGIC;
    save->version = SAVE_VERSION_CURRENT;
    for (int i = 0; i < SAVE_MAX_HIGH_SCORES; i++) {
        snprintf(save->high_scores[i].initials, SAVE_INITIALS_LENGTH, "---");
        save->high_scores[i].score = 0;
    }
    save->master_volume = 0.8f;
    save->sfx_volume = 1.0f;
    save->fullscreen = 0;
    save->control_scheme = SAVE_CONTROL_SCHEME_DEFAULT;
    save->checksum = save_checksum(save);
}

static bool migrate_save(SaveData *save) {
    if (save->version == SAVE_VERSION_CURRENT) {
        return true;
    }
    return false;
}

bool save_load(const char *path, SaveData *save) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return false;
    }

    SaveData loaded;
    size_t read_count = fread(&loaded, sizeof(loaded), 1, file);
    fclose(file);

    if (read_count != 1) {
        return false;
    }

    if (loaded.magic != SAVE_MAGIC) {
        return false;
    }

    uint32_t expected_checksum = save_checksum(&loaded);
    if (expected_checksum != loaded.checksum) {
        return false;
    }

    if (!migrate_save(&loaded)) {
        return false;
    }

    *save = loaded;
    return true;
}

bool save_write(const char *path, const SaveData *save) {
    SaveData to_write = *save;
    to_write.magic = SAVE_MAGIC;
    to_write.version = SAVE_VERSION_CURRENT;
    to_write.checksum = save_checksum(&to_write);

    char temp_path[1024];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", path);

    FILE *file = fopen(temp_path, "wb");
    if (file == NULL) {
        return false;
    }

    size_t written = fwrite(&to_write, sizeof(to_write), 1, file);
    fclose(file);

    if (written != 1) {
        remove(temp_path);
        return false;
    }

    if (rename(temp_path, path) != 0) {
        remove(temp_path);
        return false;
    }

    return true;
}

bool save_try_add_high_score(SaveData *save, uint32_t score, const char *initials) {
    int insert_index = -1;
    for (int i = 0; i < SAVE_MAX_HIGH_SCORES; i++) {
        if (score > save->high_scores[i].score) {
            insert_index = i;
            break;
        }
    }

    if (insert_index < 0) {
        return false;
    }

    for (int i = SAVE_MAX_HIGH_SCORES - 1; i > insert_index; i--) {
        save->high_scores[i] = save->high_scores[i - 1];
    }

    save->high_scores[insert_index].score = score;
    snprintf(save->high_scores[insert_index].initials, SAVE_INITIALS_LENGTH, "%s", initials);
    return true;
}
