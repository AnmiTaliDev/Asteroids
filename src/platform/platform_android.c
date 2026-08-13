#include "platform.h"

#include <SDL3/SDL.h>

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

bool platform_ensure_dir_exists(const char *path) {
    if (mkdir(path, 0700) == 0) {
        return true;
    }
    return errno == EEXIST;
}

static bool ensure_dir_recursive(char *path) {
    for (char *p = path + 1; *p != '\0'; p++) {
        if (*p == '/') {
            *p = '\0';
            if (!platform_ensure_dir_exists(path)) {
                *p = '/';
                return false;
            }
            *p = '/';
        }
    }
    return platform_ensure_dir_exists(path);
}

bool platform_file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

bool platform_init_paths(PlatformPaths *out_paths, const char *argv0) {
    (void)argv0;

    if (out_paths == NULL) {
        return false;
    }
    memset(out_paths, 0, sizeof(*out_paths));

    const char *internal_storage_path = SDL_GetAndroidInternalStoragePath();
    if (internal_storage_path == NULL) {
        SDL_Log("Failed to resolve Android internal storage path: %s", SDL_GetError());
        return false;
    }

    snprintf(out_paths->config_dir, sizeof(out_paths->config_dir), "%s", internal_storage_path);
    snprintf(out_paths->save_dir, sizeof(out_paths->save_dir), "%s", internal_storage_path);

    char config_dir_copy[PLATFORM_MAX_PATH];
    snprintf(config_dir_copy, sizeof(config_dir_copy), "%s", out_paths->config_dir);
    ensure_dir_recursive(config_dir_copy);

    out_paths->asset_dir[0] = '\0';

    return true;
}
