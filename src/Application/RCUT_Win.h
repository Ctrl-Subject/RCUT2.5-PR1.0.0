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

// Opaque handle to the single game window. RCUT_Win manages exactly one
// window at a time -- see RCUT_Win_Create.
typedef struct RCUT_Window RCUT_Window;

typedef struct RCUT_WindowDesc {
    int width;
    int height;
    const char* title;
} RCUT_WindowDesc;

// Creates the game window. Returns NULL if creation fails, OR if a window
// already exists (call RCUT_Win_Destroy first if you need to recreate it).
RCUT_API RCUT_Window* RCUT_Win_Create(const RCUT_WindowDesc* desc);

// Destroys the window. Safe to call with NULL or after already destroyed.
RCUT_API void RCUT_Win_Destroy(RCUT_Window* win);

// Processes one "tick" of OS/window events (input, resize, close requests).
// Call this once per frame, before rendering. Returns false once the
// window has been asked to close, so your main loop knows to exit.
RCUT_API bool RCUT_Win_PollEvents(RCUT_Window* win);

// Uploads an RGB framebuffer (3 bytes/pixel, row 0 = bottom of image) to
// the window and swaps buffers. Call once per frame, after rendering.
RCUT_API void RCUT_Win_Present(RCUT_Window* win, const unsigned char* rgbPixels, int width, int height);

RCUT_API int  RCUT_Win_GetWidth(const RCUT_Window* win);
RCUT_API int  RCUT_Win_GetHeight(const RCUT_Window* win);
RCUT_API bool RCUT_Win_IsOpen(const RCUT_Window* win);

#ifdef __cplusplus
}
#endif