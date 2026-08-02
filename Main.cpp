// main.cpp
// The host application. Wires the RCUT engine facade together: loads a
// map + textures, places the camera, adds a couple of test sprites, then
// runs the main loop translating input into camera movement.
//
// Deliberately includes ONLY "RCUT.h" -- pulling in any individual
// RCUT_*.h module header alongside it in this same file would redefine
// structs like RCUT_WindowDesc/RCUT_Texture/RCUT_Camera twice (they're
// fully defined in both places), which C treats as an error.

#include "RCUT.h"

#include <chrono>
#include <cstdio>

// ---------------------------------------------------------------------
// Map -- 0 = empty, 1 = wall, 2 = teleport wall
// ---------------------------------------------------------------------

static const int kMapWidth = 21;
static const int kMapHeight = 27;

static int g_map[] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,
    1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
    1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,
    1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,
    1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,
    1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,
    0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,1,
    0,0,0,0,1,0,1,0,1,1,0,1,1,0,1,0,1,0,0,0,1,
    1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,1,1,
    2,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,2,
    1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,
    0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,1,
    0,0,0,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,0,0,1,
    1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,
    1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
    1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,
    1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,
    1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,
    1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

// ---------------------------------------------------------------------
// Tuning
// ---------------------------------------------------------------------

static const float kMoveSpeed = 3.0f;          // tile units / second
static const float kRotSpeed = 2.0f;            // radians / second (arrow keys)
static const float kMouseSensitivity = 0.003f;  // radians / pixel

// ---------------------------------------------------------------------

static void LoadSceneAssets()
{
    // Paths are placeholders -- point these at whatever art you actually
    // have. Textures/audio that fail to load return -1/false; the
    // raycaster and audio modules both fall back gracefully (flat shaded
    // walls, silent audio) rather than crashing, so this will still run
    // with an empty assets/ folder -- just without art.
    RCUT_TextureId wallTex     = RCUT_Textures_Load("assets/textures/wall.png", nullptr, 1.0f);
    RCUT_TextureId teleportTex = RCUT_Textures_Load("assets/textures/teleport.png", nullptr, 1.0f);
    RCUT_TextureId floorTex    = RCUT_Textures_Load("assets/textures/floor.png", nullptr, 1.0f);
    RCUT_TextureId ceilTex     = RCUT_Textures_Load("assets/textures/ceiling.png", nullptr, 1.0f);

    // Sprites usually get exported on a solid background color rather
    // than with real per-pixel alpha -- magenta is a common choice
    // specifically because it rarely appears in actual art.
    RCUT_Color chromaKey = { 255, 0, 255 };
    RCUT_TextureId spriteTex = RCUT_Textures_Load("assets/textures/sprite.png", &chromaKey, 1.0f);

    RCUT_MapDesc mapDesc = { g_map, kMapWidth, kMapHeight };
    RCUT_Raycaster_SetMap(&mapDesc);
    RCUT_Raycaster_SetTeleportTile(2);
    RCUT_Raycaster_SetWallTexture(1, wallTex);
    RCUT_Raycaster_SetWallTexture(2, teleportTex);
    RCUT_Raycaster_SetFloorTexture(floorTex);
    RCUT_Raycaster_SetCeilingTexture(ceilTex);

    if (spriteTex >= 0) {
        RCUT_Sprite_Add(10.5f, 5.5f, spriteTex);
        RCUT_Sprite_Add(14.5f, 20.5f, spriteTex);
    }

    RCUT_Camera* cam = RCUT_Engine_GetCamera();
    RCUT_Camera_Set(cam, 2.5f, 1.5f, 0.0f, 0.66f);

    RCUT_Music_Load("assets/audio/theme.ogg");
    RCUT_Music_Play(true);
}

static void HandleInput(float dt)
{
    RCUT_Camera* cam = RCUT_Engine_GetCamera();

    if (RCUT_Input_IsSpecialKeyDown(RCUT_KEY_LEFT))  RCUT_Camera_Rotate(cam, -kRotSpeed * dt);
    if (RCUT_Input_IsSpecialKeyDown(RCUT_KEY_RIGHT)) RCUT_Camera_Rotate(cam,  kRotSpeed * dt);

    RCUT_Camera_Rotate(cam, RCUT_Input_GetMouseDeltaX() * kMouseSensitivity);

    float forward = 0.0f, strafe = 0.0f;
    if (RCUT_Input_IsKeyDown('w') || RCUT_Input_IsKeyDown('W')) forward += 1.0f;
    if (RCUT_Input_IsKeyDown('s') || RCUT_Input_IsKeyDown('S')) forward -= 1.0f;
    if (RCUT_Input_IsKeyDown('a') || RCUT_Input_IsKeyDown('A')) strafe  += 1.0f;
    if (RCUT_Input_IsKeyDown('d') || RCUT_Input_IsKeyDown('D')) strafe  -= 1.0f;

    if (forward != 0.0f || strafe != 0.0f) {
        RCUT_Raycaster_TryMove(cam, forward * kMoveSpeed * dt, strafe * kMoveSpeed * dt);
    }
}
    
int main()
{
    RCUT_EngineDesc desc = { 1024, 512, "RCUT" };
    if (!RCUT_Engine_Init(&desc)) {
        fprintf(stderr, "RCUT_Engine_Init failed\n");
        return 1;
    }

    LoadSceneAssets();

    auto lastTime = std::chrono::steady_clock::now();

    while (RCUT_Engine_BeginFrame()) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        HandleInput(dt);
        RCUT_Engine_EndFrame();
    }

    RCUT_Engine_Shutdown();
    return 0;
}