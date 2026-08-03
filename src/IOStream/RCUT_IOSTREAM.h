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

// Arrow keys and friends, via a neutral enum rather than a raw GLUT code
// -- nothing outside the Bridge should need to know GLUT's numbering.
typedef enum RCUT_Key {
    RCUT_KEY_LEFT,
    RCUT_KEY_RIGHT,
    RCUT_KEY_UP,
    RCUT_KEY_DOWN,
    RCUT_KEY_COUNT
} RCUT_Key;

// Registers input callbacks with the Bridge, hides the OS cursor, and
// confines it to the window. Call once, after RCUT_Win_Create.
RCUT_API void RCUT_Input_Init(void);
RCUT_API void RCUT_Input_Shutdown(void);

// Call once per frame, AFTER RCUT_Win_PollEvents -- finalizes this
// frame's mouse delta and resets tracking for the next frame.
RCUT_API void RCUT_Input_Update(void);

// Regular ASCII keys (letters, digits, space, etc), keyed by raw char --
// same 'w'/'a'/'s'/'d' you'd check with keys['w'] before.
RCUT_API bool RCUT_Input_IsKeyDown(unsigned char key);

RCUT_API bool RCUT_Input_IsSpecialKeyDown(RCUT_Key key);

#ifdef __cplusplus
}
#endif