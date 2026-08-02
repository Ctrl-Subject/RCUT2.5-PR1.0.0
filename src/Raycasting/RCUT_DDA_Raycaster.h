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
#include "RCUT_Objects.h"
#include "RCUT_Textures.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------

// Allocates the framebuffer/z-buffer at this resolution. Call once
// before rendering. Should generally match your RCUT_Window's size.
RCUT_API bool RCUT_Raycaster_Init(int screenWidth, int screenHeight);
RCUT_API void RCUT_Raycaster_Shutdown(void);

// ---------------------------------------------------------------------
// Map
// ---------------------------------------------------------------------
// The raycaster is the only module that knows what a "wall" is -- this
// is also why collision-checked movement (RCUT_Raycaster_TryMove) lives
// here instead of in RCUT_Objects.

typedef struct RCUT_MapDesc {
    const int* tiles; // row-major, tiles[y*width+x], 0 = empty/walkable
    int width;
    int height;
} RCUT_MapDesc;

// Copies the tile grid in. Any non-zero tile value is a solid wall
// UNLESS it's been registered via RCUT_Raycaster_SetTeleportTile.
RCUT_API void RCUT_Raycaster_SetMap(const RCUT_MapDesc* desc);

// Binds a tile value (1, 2, 3...) to a texture so that wall renders
// textured. Untextured tile values fall back to flat shading.
RCUT_API void RCUT_Raycaster_SetWallTexture(int tileValue, RCUT_TextureId textureId);

// Marks a tile value as a teleport tile: wraps the camera to the
// opposite edge of the map instead of blocking it like a normal wall.
// It still RENDERS as a wall (textured via SetWallTexture same as any
// other tile value) -- only collision behavior differs.
RCUT_API void RCUT_Raycaster_SetTeleportTile(int tileValue);

// Optional -- floor/ceiling render as flat shaded colors if never set.
RCUT_API void RCUT_Raycaster_SetFloorTexture(RCUT_TextureId textureId);
RCUT_API void RCUT_Raycaster_SetCeilingTexture(RCUT_TextureId textureId);

// ---------------------------------------------------------------------
// Collision-checked camera movement
// ---------------------------------------------------------------------
// Resolves against the map (walls block, teleport tiles wrap) and
// commits the result directly into *cam. Checked per-axis so sliding
// along a wall works instead of movement just stopping dead.
//
// forwardAmount/strafeAmount are in world/tile units, may be negative,
// and are typically speed * deltaTime from your game loop.
RCUT_API void RCUT_Raycaster_TryMove(RCUT_Camera* cam, float forwardAmount, float strafeAmount);

// ---------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------
// Renders floor, ceiling, walls, and every live sprite from
// RCUT_Sprite_GetAll() into the internal framebuffer, as seen from *cam.
RCUT_API void RCUT_Raycaster_Render(const RCUT_Camera* cam);

// RGB, 3 bytes/pixel, row 0 = bottom of image -- pass straight to
// RCUT_Win_Present. Owned by the raycaster; do not free.
RCUT_API const unsigned char* RCUT_Raycaster_GetFramebuffer(void);
RCUT_API int RCUT_Raycaster_GetWidth(void);
RCUT_API int RCUT_Raycaster_GetHeight(void);

#ifdef __cplusplus
}
#endif