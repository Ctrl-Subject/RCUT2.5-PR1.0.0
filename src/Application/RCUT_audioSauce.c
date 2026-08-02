#include "RCUT_audioSauce.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <stdlib.h>
#include <string.h>

#define MAX_VOICES_PER_SFX 4

typedef struct {
    ma_sound sound;
    bool initialized; // has ma_sound_init_from_file succeeded for this voice yet
} Voice;

typedef struct {
    char path[260];
    Voice voices[MAX_VOICES_PER_SFX];
    int nextVoice; // round-robin cursor
    bool inUse;
} SFXSlot;

static ma_engine g_engine;
static bool g_engineReady = false;

static SFXSlot* g_sfx = NULL;
static int g_sfxCount = 0;
static int g_sfxCapacity = 0;

static ma_sound g_music;
static bool g_musicLoaded = false;

// ---------------------------------------------------------------------

bool RCUT_Audio_Init(void)
{
    if (ma_engine_init(NULL, &g_engine) != MA_SUCCESS) return false;
    g_engineReady = true;
    return true;
}

void RCUT_Audio_Shutdown(void)
{
    if (!g_engineReady) return;

    for (int i = 0; i < g_sfxCount; i++) {
        for (int v = 0; v < MAX_VOICES_PER_SFX; v++) {
            if (g_sfx[i].voices[v].initialized) {
                ma_sound_uninit(&g_sfx[i].voices[v].sound);
            }
        }
    }
    free(g_sfx);
    g_sfx = NULL;
    g_sfxCount = g_sfxCapacity = 0;

    if (g_musicLoaded) {
        ma_sound_uninit(&g_music);
        g_musicLoaded = false;
    }

    ma_engine_uninit(&g_engine);
    g_engineReady = false;
}

// ---------------------------------------------------------------------
// SFX
// ---------------------------------------------------------------------

static int FindFreeSFXSlot(void)
{
    for (int i = 0; i < g_sfxCount; i++) {
        if (!g_sfx[i].inUse) return i;
    }
    if (g_sfxCount == g_sfxCapacity) {
        g_sfxCapacity = g_sfxCapacity ? g_sfxCapacity * 2 : 8;
        g_sfx = (SFXSlot*)realloc(g_sfx, sizeof(SFXSlot) * g_sfxCapacity);
    }
    return g_sfxCount++;
}

RCUT_SoundId RCUT_Audio_LoadSFX(const char* path)
{
    if (!g_engineReady || !path) return -1;

    // Validate the file up front (rather than discovering a bad path the
    // first time something tries to play it) by test-loading one voice.
    ma_sound testSound;
    if (ma_sound_init_from_file(&g_engine, path, MA_SOUND_FLAG_DECODE, NULL, NULL, &testSound) != MA_SUCCESS) {
        return -1;
    }

    int slot = FindFreeSFXSlot();
    SFXSlot* s = &g_sfx[slot];
    memset(s, 0, sizeof(SFXSlot));
    strncpy(s->path, path, sizeof(s->path) - 1);
    s->inUse = true;
    s->voices[0].sound = testSound;
    s->voices[0].initialized = true; // reuse the one we just validated as voice 0

    return (RCUT_SoundId)slot;
}

void RCUT_Audio_PlaySFX(RCUT_SoundId id, float volume)
{
    if (!g_engineReady || id < 0 || id >= g_sfxCount || !g_sfx[id].inUse) return;
    SFXSlot* s = &g_sfx[id];

    int v = s->nextVoice;
    s->nextVoice = (s->nextVoice + 1) % MAX_VOICES_PER_SFX;
    Voice* voice = &s->voices[v];

    if (!voice->initialized) {
        if (ma_sound_init_from_file(&g_engine, s->path, MA_SOUND_FLAG_DECODE, NULL, NULL, &voice->sound) != MA_SUCCESS) {
            return;
        }
        voice->initialized = true;
    } else {
        ma_sound_stop(&voice->sound); // cut short if this voice was still playing
    }

    ma_sound_seek_to_pcm_frame(&voice->sound, 0);
    ma_sound_set_volume(&voice->sound, volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume));
    ma_sound_start(&voice->sound);
}

// ---------------------------------------------------------------------
// Music
// ---------------------------------------------------------------------

bool RCUT_Music_Load(const char* path)
{
    if (!g_engineReady || !path) return false;

    if (g_musicLoaded) {
        ma_sound_uninit(&g_music);
        g_musicLoaded = false;
    }

    if (ma_sound_init_from_file(&g_engine, path, MA_SOUND_FLAG_STREAM, NULL, NULL, &g_music) != MA_SUCCESS) {
        return false;
    }
    g_musicLoaded = true;
    return true;
}

void RCUT_Music_Play(bool loop)
{
    if (!g_musicLoaded) return;
    ma_sound_set_looping(&g_music, loop ? MA_TRUE : MA_FALSE);
    ma_sound_start(&g_music);
}

void RCUT_Music_Stop(void)
{
    if (!g_musicLoaded) return;
    ma_sound_stop(&g_music);
    ma_sound_seek_to_pcm_frame(&g_music, 0);
}

void RCUT_Music_Pause(void)
{
    if (!g_musicLoaded) return;
    ma_sound_stop(&g_music); // miniaudio's stop preserves playback position
}

void RCUT_Music_Resume(void)
{
    if (!g_musicLoaded) return;
    ma_sound_start(&g_music);
}

void RCUT_Music_SetVolume(float volume)
{
    if (!g_musicLoaded) return;
    ma_sound_set_volume(&g_music, volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume));
}

bool RCUT_Music_IsPlaying(void)
{
    if (!g_musicLoaded) return false;
    return ma_sound_is_playing(&g_music) == MA_TRUE;
}