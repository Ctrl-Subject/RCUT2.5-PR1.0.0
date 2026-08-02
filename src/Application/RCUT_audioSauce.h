#pragma once

#ifndef RCUT_API
# if defined(RCUT_STATIC)
#  define RCUT_API
# elif defined(RCUT_BUILD_DLL)
#  if defined(_WIN32) || defined(__CYGWIN__)
#   define RCUT_API __declspec(dllexport)
#  else
#   define RCUT_API
#  endif
# elif defined(RCUT_USE_DLL)
#  if defined(_WIN32) || defined(__CYGWIN__)
#   define RCUT_API __declspec(dllimport)
#  else
#   define RCUT_API
#  endif
# else
#  define RCUT_API
# endif
#endif

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int RCUT_SoundId; // -1 = invalid

RCUT_API bool RCUT_Audio_Init(void);
RCUT_API void RCUT_Audio_Shutdown(void);

// Preloads/validates a sound file (WAV/MP3/OGG via miniaudio). Returns an
// id, or -1 if the file couldn't be found/decoded.
RCUT_API RCUT_SoundId RCUT_Audio_LoadSFX(const char* path);

// Fires an overlapping playback instance of a preloaded SFX -- safe to
// call the same id repeatedly in quick succession (rapid footsteps,
// several teleports going off close together, etc). volume is 0..1.
//
// Each loaded SFX has a small fixed pool of voices (see MAX_VOICES_PER_SFX
// in the .c file); if you retrigger faster than the pool size, the oldest
// still-playing voice gets cut short to make room -- fine for footsteps,
// might matter for something like a wall of overlapping explosions.
RCUT_API void RCUT_Audio_PlaySFX(RCUT_SoundId id, float volume);

// --- Background music: one persistent, loopable channel ---
// Deliberately separate from the SFX pool -- music needs pause/resume/
// volume control over a long-lived instance, not fire-and-forget voices.

RCUT_API bool RCUT_Music_Load(const char* path); // replaces any currently loaded track
RCUT_API void RCUT_Music_Play(bool loop);
RCUT_API void RCUT_Music_Stop(void);
RCUT_API void RCUT_Music_Pause(void);
RCUT_API void RCUT_Music_Resume(void);
RCUT_API void RCUT_Music_SetVolume(float volume); // 0..1
RCUT_API bool RCUT_Music_IsPlaying(void);

#ifdef __cplusplus
}
#endif