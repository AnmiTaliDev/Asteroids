# Asteroids

A classic vector-style Asteroids arcade game written in C17 with SDL3.

## Building

### Requirements

- CMake 3.20+
- A C17 compiler (GCC or Clang)
- SDL3 (development package), discoverable via CMake's `find_package(SDL3 CONFIG)`

On Arch Linux: `pacman -S cmake gcc sdl3`

On Debian/Ubuntu, SDL3 is not yet packaged in most releases; build it from source
(see the CI workflow in `.github/workflows/ci.yml` for a working recipe).

### Build steps

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j
./build/src/asteroids
```

### CMake options

| Option | Default | Description |
| --- | --- | --- |
| `ASTEROIDS_ENABLE_DEBUG_OVERLAY` | `ON` | Compiles in the F3 debug overlay (FPS, asteroid count, hitboxes). |
| `ASTEROIDS_ENABLE_ASSERTIONS` | `OFF` | Forces runtime assertions on regardless of build type. |
| `ASTEROIDS_ALTERNATE_CONTROLS` | `OFF` | Builds with arrow-key rotation/thrust instead of the default WASD scheme. |
| `ASTEROIDS_WARNINGS_AS_ERRORS` | `OFF` | Treats compiler warnings as build errors. |
| `ASTEROIDS_BUILD_TESTS` | `OFF` | Builds the unit test suite (if present). |

Example: `cmake -S . -B build -DASTEROIDS_ALTERNATE_CONTROLS=ON`

### Android

The `android/` directory is a Gradle project that builds the same `src/` sources
(via `android/app/jni/CMakeLists.txt`, which `add_subdirectory()`s this repository's
root `CMakeLists.txt`) into `libmain.so`, loaded by SDL3's stock `SDLActivity` Java
glue (vendored under `android/app/src/main/java/org/libsdl/app/`, from SDL3 itself,
zlib-licensed; see `SDL3-LICENSE.txt` next to it). No custom Java code is needed.

SDL3 and SDL3_ttf are git submodules at `android/app/jni/SDL` and
`android/app/jni/SDL_ttf`, pinned to `release-3.4.14` and `release-3.2.2`
respectively (matching what CI builds from source for desktop). SDL3_ttf pulls in
its own nested freetype submodule (harfbuzz and plutosvg are disabled since this
game only needs plain Latin glyphs). After cloning this repository:

```sh
git submodule update --init --recursive android/app/jni/SDL android/app/jni/SDL_ttf
cd android
./gradlew assembleDebug
```

This requires the Android SDK (API 35) and NDK (tested with r28c) to be installed,
either via Android Studio or the command-line `sdkmanager`. The resulting APK is a
debug build for `arm64-v8a` targeting a minimum of Android 8.0 (API 26); adjust
`abiFilters` and `minSdkVersion` in `android/app/build.gradle` if you need broader
device coverage. Config and save data live in the app's private internal storage on
Android (see the Save data section below); sound assets are bundled into the APK
from this repository's `assets/` directory.

## Controls

| Action | Default scheme | Alternate scheme (`ASTEROIDS_ALTERNATE_CONTROLS=ON`) |
| --- | --- | --- |
| Rotate left/right | A / D | Left / Right arrow |
| Thrust | W | Up arrow |
| Fire | Space | Space |
| Pause | P or Escape | P or Escape |
| Start / confirm | Enter | Enter |
| Open options menu | O (from main menu or pause) | O (from main menu or pause) |
| Toggle debug overlay | F3 (if compiled in) | F3 (if compiled in) |

The `ASTEROIDS_ALTERNATE_CONTROLS` CMake option only picks the *default* control
scheme used the first time the game runs; from then on the choice lives in your save
data and can be changed at any time from the in-game options menu (see below).

## Configuration

Two layers of configuration exist, for two different audiences:

- **In-game options menu**: press `O` from the main menu or the pause screen to open
  a settings screen where you can adjust master volume, sound effect volume,
  fullscreen, and the control scheme (WASD or arrow keys) with the arrow keys/WASD to
  navigate and Enter/Left/Right to change values, without recompiling or editing any
  files. These choices are written to your save data immediately.
- **External config file**: deeper gameplay tuning (ship handling, weapon behavior,
  asteroid sizes/speeds/scoring, wave difficulty, lives, starfield density, window
  size) lives in a text config file the game writes to your user config directory on
  first launch (`$XDG_CONFIG_HOME/asteroids/game.cfg`, falling back to
  `~/.config/asteroids/game.cfg`). Edit this file to tune gameplay without
  recompiling; delete it to regenerate the defaults. A reference copy documenting
  every option lives at `config/game.cfg` in this repository.

## Save data

High scores and the in-game options (audio volumes, fullscreen, control scheme)
persist in a versioned binary save file at
`$XDG_DATA_HOME/asteroids/savegame.dat` (falling back to
`~/.local/share/asteroids/savegame.dat`). The format includes a magic number, format
version, and checksum; saves that fail validation are safely ignored and regenerated
from defaults rather than corrupting game state.

## Assets

- Sprites are vector line shapes defined in code (`src/core/shapes.h`), not image files.
- Sound effects are procedurally synthesized WAV files, committed under `assets/sounds/`.
- Text is rendered with [Press Start 2P](https://fonts.google.com/specimen/Press+Start+2P)
  via SDL_ttf (`assets/fonts/PressStart2P-Regular.ttf`), a third-party font licensed
  under the SIL Open Font License 1.1 (see `assets/fonts/PressStart2P-OFL.txt`), not
  under this project's own asset license.

The sprites and sound effects, which are original to this project, are licensed under
CC BY-SA 4.0 (see `assets/LICENSE`). The bundled font keeps its own OFL license.

## Portability design

The only Linux-specific code lives behind `src/platform/platform.h`, in
`src/platform/platform_linux.c` (config/save/asset path resolution using XDG base
directories). Game logic, rendering, audio, and config/save handling use only C17
standard library facilities and SDL3, none of which are Linux-specific. Porting to
another OS means implementing `src/platform/platform_<os>.c` against the existing
`PlatformPaths` abstraction and pointing CMake at it.

## License

- Code is licensed under the GNU General Public License v3.0. See `LICENSE`.
- Assets are licensed under Creative Commons Attribution-ShareAlike 4.0. See `assets/LICENSE`.
