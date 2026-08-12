#ifndef ASTEROIDS_AUDIO_AUDIO_H
#define ASTEROIDS_AUDIO_AUDIO_H

#include <stdbool.h>
#include <SDL3/SDL.h>

typedef enum SoundId {
    SOUND_FIRE,
    SOUND_THRUST,
    SOUND_EXPLOSION_LARGE,
    SOUND_EXPLOSION_MEDIUM,
    SOUND_EXPLOSION_SMALL,
    SOUND_SHIP_DESTROYED,
    SOUND_EXTRA_LIFE,
    SOUND_COUNT
} SoundId;

typedef struct SoundClip {
    Uint8 *buffer;
    Uint32 length;
    SDL_AudioSpec spec;
    bool loaded;
} SoundClip;

typedef struct AudioSystem {
    SDL_AudioDeviceID device_id;
    SoundClip clips[SOUND_COUNT];
    SDL_AudioStream *streams[SOUND_COUNT];
    bool thrust_playing;
    float master_volume;
    float sfx_volume;
} AudioSystem;

bool audio_init(AudioSystem *audio, const char *asset_dir, float master_volume, float sfx_volume);
void audio_shutdown(AudioSystem *audio);
void audio_play(AudioSystem *audio, SoundId sound_id);
void audio_set_thrust_playing(AudioSystem *audio, bool playing);
void audio_update(AudioSystem *audio);

#endif
