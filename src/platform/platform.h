#ifndef ASTEROIDS_PLATFORM_PLATFORM_H
#define ASTEROIDS_PLATFORM_PLATFORM_H

#include <stdbool.h>

#define PLATFORM_MAX_PATH 1024

typedef struct PlatformPaths {
    char config_dir[PLATFORM_MAX_PATH];
    char save_dir[PLATFORM_MAX_PATH];
    char asset_dir[PLATFORM_MAX_PATH];
} PlatformPaths;

bool platform_init_paths(PlatformPaths *out_paths, const char *argv0);
bool platform_ensure_dir_exists(const char *path);
bool platform_file_exists(const char *path);

#endif
