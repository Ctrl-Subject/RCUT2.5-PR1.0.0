#include "RCUT_IOSTREAM.h"
#include "RCUT_freeGlut_Bridge.h"

#include <string.h>

static bool g_keys[256];
static bool g_specialKeys[RCUT_KEY_COUNT];

static int g_windowCenterX = 0, g_windowCenterY = 0;
static float g_mouseDeltaX = 0.0f, g_mouseDeltaY = 0.0f;
static float g_pendingDeltaX = 0.0f, g_pendingDeltaY = 0.0f;

// True right after WE warp the pointer ourselves -- lets us tell "the
// user moved the mouse" apart from "the recenter warp just fired a
// synthetic move event", which would otherwise feed back into itself.
static bool g_ignoreNextMove = false;

static void OnKeyDown(unsigned char key) { g_keys[key] = true; }
static void OnKeyUp(unsigned char key)   { g_keys[key] = false; }

static RCUT_Key TranslateSpecialKey(RCUT_Bridge_SpecialKey key)
{
    switch (key) {
        case RCUT_BRIDGE_KEY_LEFT:  return RCUT_KEY_LEFT;
        case RCUT_BRIDGE_KEY_RIGHT: return RCUT_KEY_RIGHT;
        case RCUT_BRIDGE_KEY_UP:    return RCUT_KEY_UP;
        case RCUT_BRIDGE_KEY_DOWN:  return RCUT_KEY_DOWN;
        default:                    return RCUT_KEY_COUNT; // sentinel = "none"
    }
}

static void OnSpecialDown(RCUT_Bridge_SpecialKey key) { RCUT_Key k = TranslateSpecialKey(key); if (k != RCUT_KEY_COUNT) g_specialKeys[k] = true; }
static void OnSpecialUp(RCUT_Bridge_SpecialKey key)   { RCUT_Key k = TranslateSpecialKey(key); if (k != RCUT_KEY_COUNT) g_specialKeys[k] = false; }

static void OnMouseMove(int x, int y)
{
    if (g_ignoreNextMove) {
        g_ignoreNextMove = false;
        return;
    }

    g_pendingDeltaX += (float)(x - g_windowCenterX);
    g_pendingDeltaY += (float)(y - g_windowCenterY);

    g_ignoreNextMove = true;
    RCUT_Bridge_WarpPointer(g_windowCenterX, g_windowCenterY);
}

void RCUT_Input_Init(void)
{
    memset(g_keys, 0, sizeof(g_keys));
    memset(g_specialKeys, 0, sizeof(g_specialKeys));
    g_mouseDeltaX = g_mouseDeltaY = 0.0f;
    g_pendingDeltaX = g_pendingDeltaY = 0.0f;
    g_ignoreNextMove = false;

    g_windowCenterX = RCUT_Bridge_GetWidth() / 2;
    g_windowCenterY = RCUT_Bridge_GetHeight() / 2;

    RCUT_Bridge_SetKeyCallbacks(OnKeyDown, OnKeyUp);
    RCUT_Bridge_SetSpecialKeyCallbacks(OnSpecialDown, OnSpecialUp);
    RCUT_Bridge_SetMouseMoveCallback(OnMouseMove);

    RCUT_Bridge_SetCursorVisible(false);
    RCUT_Bridge_ConfineCursorToWindow();
}

void RCUT_Input_Shutdown(void)
{
    RCUT_Bridge_SetKeyCallbacks(NULL, NULL);
    RCUT_Bridge_SetSpecialKeyCallbacks(NULL, NULL);
    RCUT_Bridge_SetMouseMoveCallback(NULL);
    RCUT_Bridge_SetCursorVisible(true);
}

void RCUT_Input_Update(void)
{
    // Keep the recenter target in sync in case the window was resized.
    g_windowCenterX = RCUT_Bridge_GetWidth() / 2;
    g_windowCenterY = RCUT_Bridge_GetHeight() / 2;

    g_mouseDeltaX = g_pendingDeltaX;
    g_mouseDeltaY = g_pendingDeltaY;
    g_pendingDeltaX = g_pendingDeltaY = 0.0f;
}

bool RCUT_Input_IsKeyDown(unsigned char key) { return g_keys[key]; }

bool RCUT_Input_IsSpecialKeyDown(RCUT_Key key)
{
    if (key < 0 || key >= RCUT_KEY_COUNT) return false;
    return g_specialKeys[key];
}

float RCUT_Input_GetMouseDeltaX(void) { return g_mouseDeltaX; }
float RCUT_Input_GetMouseDeltaY(void) { return g_mouseDeltaY; }