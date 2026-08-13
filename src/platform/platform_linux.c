#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>
#include <limits.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif

static void join_path(char *out, size_t out_size, const char *base, const char *suffix) {
    snprintf(out, out_size, "%s/%s", base, suffix);
}

static bool resolve_executable_dir(char *out, size_t out_size, const char *argv0) {
    char resolved[PATH_MAX];
    if (realpath("/proc/self/exe", resolved) != NULL) {
        char *dir = dirname(resolved);
        snprintf(out, out_size, "%s", dir);
        return true;
    }

    if (argv0 != NULL) {
        char buffer[PATH_MAX];
        snprintf(buffer, sizeof(buffer), "%s", argv0);
        char *dir = dirname(buffer);
        if (realpath(dir, resolved) != NULL) {
            snprintf(out, out_size, "%s", resolved);
            return true;
        }
    }

    return false;
}

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
    if (out_paths == NULL) {
        return false;
    }
    memset(out_paths, 0, sizeof(*out_paths));

    const char *home = getenv("HOME");
    const char *xdg_config = getenv("XDG_CONFIG_HOME");
    const char *xdg_data = getenv("XDG_DATA_HOME");

    char config_base[PLATFORM_MAX_PATH];
    char data_base[PLATFORM_MAX_PATH];

    if (xdg_config != NULL && xdg_config[0] != '\0') {
        snprintf(config_base, sizeof(config_base), "%s", xdg_config);
    } else if (home != NULL) {
        join_path(config_base, sizeof(config_base), home, ".config");
    } else {
        snprintf(config_base, sizeof(config_base), ".");
    }

    if (xdg_data != NULL && xdg_data[0] != '\0') {
        snprintf(data_base, sizeof(data_base), "%s", xdg_data);
    } else if (home != NULL) {
        join_path(data_base, sizeof(data_base), home, ".local/share");
    } else {
        snprintf(data_base, sizeof(data_base), ".");
    }

    join_path(out_paths->config_dir, sizeof(out_paths->config_dir), config_base, "asteroids");
    join_path(out_paths->save_dir, sizeof(out_paths->save_dir), data_base, "asteroids");

    char config_dir_copy[PLATFORM_MAX_PATH];
    char save_dir_copy[PLATFORM_MAX_PATH];
    snprintf(config_dir_copy, sizeof(config_dir_copy), "%s", out_paths->config_dir);
    snprintf(save_dir_copy, sizeof(save_dir_copy), "%s", out_paths->save_dir);
    ensure_dir_recursive(config_dir_copy);
    ensure_dir_recursive(save_dir_copy);

    char exe_dir[PLATFORM_MAX_PATH];
    if (resolve_executable_dir(exe_dir, sizeof(exe_dir), argv0)) {
        char candidate[PLATFORM_MAX_PATH];
        join_path(candidate, sizeof(candidate), exe_dir, "assets");
        if (platform_file_exists(candidate)) {
            snprintf(out_paths->asset_dir, sizeof(out_paths->asset_dir), "%s", candidate);
            return true;
        }

        join_path(candidate, sizeof(candidate), exe_dir, "../assets");
        if (platform_file_exists(candidate)) {
            snprintf(out_paths->asset_dir, sizeof(out_paths->asset_dir), "%s", candidate);
            return true;
        }

        join_path(candidate, sizeof(candidate), exe_dir, "../share/asteroids/assets");
        if (platform_file_exists(candidate)) {
            snprintf(out_paths->asset_dir, sizeof(out_paths->asset_dir), "%s", candidate);
            return true;
        }
    }

#ifdef ASTEROIDS_INSTALL_ASSET_DIR
    if (platform_file_exists(ASTEROIDS_INSTALL_ASSET_DIR)) {
        snprintf(out_paths->asset_dir, sizeof(out_paths->asset_dir), "%s", ASTEROIDS_INSTALL_ASSET_DIR);
        return true;
    }
#endif

    snprintf(out_paths->asset_dir, sizeof(out_paths->asset_dir), "assets");
    return true;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
