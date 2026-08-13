#include "audio.h"

#include <stdio.h>
#include <string.h>

static const char *SOUND_FILE_NAMES[SOUND_COUNT] = {
    "fire.wav",
    "thrust.wav",
    "explosion_large.wav",
    "explosion_medium.wav",
    "explosion_small.wav",
    "ship_destroyed.wav",
    "extra_life.wav",
};

static bool load_clip(SoundClip *clip, const char *asset_dir, const char *file_name) {
    char path[1024];
    if (asset_dir[0] == '\0') {
        snprintf(path, sizeof(path), "sounds/%s", file_name);
    } else {
        snprintf(path, sizeof(path), "%s/sounds/%s", asset_dir, file_name);
    }

    if (!SDL_LoadWAV(path, &clip->spec, &clip->buffer, &clip->length)) {
        SDL_Log("Failed to load sound '%s': %s", path, SDL_GetError());
        return false;
    }

    clip->loaded = true;
    return true;
}

bool audio_init(AudioSystem *audio, const char *asset_dir, float master_volume, float sfx_volume) {
    memset(audio, 0, sizeof(*audio));
    audio->master_volume = master_volume;
    audio->sfx_volume = sfx_volume;

    audio->device_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (audio->device_id == 0) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
        return false;
    }

    for (int i = 0; i < SOUND_COUNT; i++) {
        if (!load_clip(&audio->clips[i], asset_dir, SOUND_FILE_NAMES[i])) {
            continue;
        }

        audio->streams[i] = SDL_CreateAudioStream(&audio->clips[i].spec, NULL);
        if (audio->streams[i] == NULL) {
            SDL_Log("Failed to create audio stream: %s", SDL_GetError());
            continue;
        }

        SDL_BindAudioStream(audio->device_id, audio->streams[i]);
    }

    return true;
}

void audio_shutdown(AudioSystem *audio) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (audio->streams[i] != NULL) {
            SDL_DestroyAudioStream(audio->streams[i]);
            audio->streams[i] = NULL;
        }
        if (audio->clips[i].loaded) {
            SDL_free(audio->clips[i].buffer);
            audio->clips[i].loaded = false;
        }
    }

    if (audio->device_id != 0) {
        SDL_CloseAudioDevice(audio->device_id);
        audio->device_id = 0;
    }
}

void audio_play(AudioSystem *audio, SoundId sound_id) {
    if (sound_id < 0 || sound_id >= SOUND_COUNT) {
        return;
    }
    SoundClip *clip = &audio->clips[sound_id];
    SDL_AudioStream *stream = audio->streams[sound_id];
    if (!clip->loaded || stream == NULL) {
        return;
    }

    float volume = audio->master_volume * audio->sfx_volume;
    SDL_SetAudioStreamGain(stream, volume);

    SDL_ClearAudioStream(stream);
    SDL_PutAudioStreamData(stream, clip->buffer, (int)clip->length);
}

void audio_set_thrust_playing(AudioSystem *audio, bool playing) {
    audio->thrust_playing = playing;
    if (!playing) {
        SDL_AudioStream *stream = audio->streams[SOUND_THRUST];
        if (stream != NULL) {
            SDL_ClearAudioStream(stream);
        }
    }
}

void audio_update(AudioSystem *audio) {
    if (!audio->thrust_playing) {
        return;
    }

    SoundClip *clip = &audio->clips[SOUND_THRUST];
    SDL_AudioStream *stream = audio->streams[SOUND_THRUST];
    if (!clip->loaded || stream == NULL) {
        return;
    }

    float volume = audio->master_volume * audio->sfx_volume;
    SDL_SetAudioStreamGain(stream, volume);

    int queued = SDL_GetAudioStreamQueued(stream);
    if (queued < (int)clip->length) {
        SDL_PutAudioStreamData(stream, clip->buffer, (int)clip->length);
    }
}
