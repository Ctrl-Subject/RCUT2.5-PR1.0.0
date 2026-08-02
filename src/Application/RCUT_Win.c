#include "RCUT_Win.h"
#include "RCUT_freeGlut_Bridge.h"

#include <stdlib.h>

struct RCUT_Window {
    int  width;
    int  height;
    bool open;
};

// Single-window rule: only one instance may exist at a time.
static RCUT_Window* g_activeWindow = NULL;

RCUT_Window* RCUT_Win_Create(const RCUT_WindowDesc* desc)
{
    if (g_activeWindow != NULL) return NULL; // already have a window
    if (!desc || desc->width <= 0 || desc->height <= 0) return NULL;

    const char* title = desc->title ? desc->title : "RCUT";
    if (!RCUT_Bridge_CreateWindow(desc->width, desc->height, title)) {
        return NULL;
    }

    RCUT_Window* win = (RCUT_Window*)malloc(sizeof(RCUT_Window));
    if (!win) {
        RCUT_Bridge_DestroyWindow();
        return NULL;
    }

    win->width  = desc->width;
    win->height = desc->height;
    win->open   = true;

    g_activeWindow = win;
    return win;
}

void RCUT_Win_Destroy(RCUT_Window* win)
{
    if (!win) return;
    RCUT_Bridge_DestroyWindow();
    if (g_activeWindow == win) g_activeWindow = NULL;
    free(win);
}

bool RCUT_Win_PollEvents(RCUT_Window* win)
{
    if (!win || !win->open) return false;

    RCUT_Bridge_PumpEvents();

    if (RCUT_Bridge_ShouldClose()) {
        win->open = false;
    }

    // Keep cached size in sync in case the Bridge detected a resize.
    win->width  = RCUT_Bridge_GetWidth();
    win->height = RCUT_Bridge_GetHeight();

    return win->open;
}

void RCUT_Win_Present(RCUT_Window* win, const unsigned char* rgbPixels, int width, int height)
{
    if (!win || !win->open || !rgbPixels) return;
    RCUT_Bridge_SwapAndBlit(rgbPixels, width, height);
}

int  RCUT_Win_GetWidth(const RCUT_Window* win)  { return win ? win->width  : 0; }
int  RCUT_Win_GetHeight(const RCUT_Window* win) { return win ? win->height : 0; }
bool RCUT_Win_IsOpen(const RCUT_Window* win)    { return win ? win->open   : false; }

