#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Low-level freeglut wrapper. RCUT_Win is the only module that should
// call these -- nothing else in the engine should include this header
// or need to know freeglut exists underneath.

bool RCUT_Bridge_CreateWindow(int width, int height, const char* title);
void RCUT_Bridge_DestroyWindow(void);

// Processes one non-blocking tick of pending OS/window events (resize,
// close requests, etc). Uses freeglut's glutMainLoopEvent(), NOT
// glutMainLoop() -- the latter never returns, which doesn't fit a
// polling-style game loop like RCUT_Win_PollEvents expects.
void RCUT_Bridge_PumpEvents(void);

// True once the user has clicked the window's close button.
bool RCUT_Bridge_ShouldClose(void);

// Draws an RGB buffer (row 0 = bottom of image) to the window and swaps
// buffers. Safe to call any time after RCUT_Bridge_CreateWindow.
void RCUT_Bridge_SwapAndBlit(const unsigned char* rgbPixels, int width, int height);

// Neutral special-key enum so nothing above the Bridge needs GLUT_KEY_*.
typedef enum RCUT_Bridge_SpecialKey {
    RCUT_BRIDGE_KEY_UNKNOWN = 0,
    RCUT_BRIDGE_KEY_LEFT,
    RCUT_BRIDGE_KEY_RIGHT,
    RCUT_BRIDGE_KEY_UP,
    RCUT_BRIDGE_KEY_DOWN
} RCUT_Bridge_SpecialKey;

// Pass NULL for any callback to unregister it.
void RCUT_Bridge_SetKeyCallbacks(void (*onKeyDown)(unsigned char key), void (*onKeyUp)(unsigned char key));
void RCUT_Bridge_SetSpecialKeyCallbacks(void (*onSpecialDown)(RCUT_Bridge_SpecialKey key), void (*onSpecialUp)(RCUT_Bridge_SpecialKey key));
void RCUT_Bridge_SetMouseMoveCallback(void (*onMouseMove)(int x, int y));

void RCUT_Bridge_WarpPointer(int x, int y);
void RCUT_Bridge_SetCursorVisible(bool visible);
void RCUT_Bridge_ConfineCursorToWindow(void); // Win32 ClipCursor, matches your original behavior

int RCUT_Bridge_GetWidth(void);
int RCUT_Bridge_GetHeight(void);

#ifdef __cplusplus
}
#endif