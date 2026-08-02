#include "RCUT.h"

static RCUT_Window* g_window = NULL;
static RCUT_Camera g_camera;
static bool g_initialized = false;

bool RCUT_Engine_Init(const RCUT_EngineDesc* desc)
{
    if (g_initialized || !desc) return false;

    RCUT_WindowDesc winDesc = { desc->width, desc->height, desc->title };
    g_window = RCUT_Win_Create(&winDesc);
    if (!g_window) return false;

    if (!RCUT_Raycaster_Init(desc->width, desc->height)) {
        RCUT_Win_Destroy(g_window);
        g_window = NULL;
        return false;
    }

    // Input needs the Bridge's window/callbacks already live, which
    // RCUT_Win_Create just set up -- this order matters.
    RCUT_Input_Init();

    if (!RCUT_Audio_Init()) {
        // Non-fatal: a game without sound still runs. Flag it, don't fail.
        // (swap for your own logging once RCUT has one)
    }

    RCUT_Camera_Set(&g_camera, 0.0f, 0.0f, 0.0f, 0.66f);

    g_initialized = true;
    return true;
}

void RCUT_Engine_Shutdown(void)
{
    if (!g_initialized) return;

    RCUT_Audio_Shutdown();
    RCUT_Input_Shutdown();
    RCUT_Raycaster_Shutdown();
    RCUT_Win_Destroy(g_window);
    g_window = NULL;

    g_initialized = false;
}

bool RCUT_Engine_BeginFrame(void)
{
    if (!g_initialized) return false;

    bool stillOpen = RCUT_Win_PollEvents(g_window);
    RCUT_Input_Update();

    return stillOpen;
}

void RCUT_Engine_EndFrame(void)
{
    if (!g_initialized) return;

    RCUT_Raycaster_Render(&g_camera);
    RCUT_Win_Present(g_window,
                      RCUT_Raycaster_GetFramebuffer(),
                      RCUT_Raycaster_GetWidth(),
                      RCUT_Raycaster_GetHeight());
}

RCUT_Camera* RCUT_Engine_GetCamera(void)
{
    return g_initialized ? &g_camera : NULL;
}