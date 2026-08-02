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

#include "RCUT_Textures.h"

// ---------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------
// Single camera, matching RCUT_Win's single-window rule. Position and
// facing are stored as dir/plane vectors (not just an angle) since the
// raycaster needs both for wall casting AND for projecting sprites into
// screen space -- deriving plane from an angle every frame would just be
// redundant trig for something the raycaster already needs every column.
//
// RCUT_Objects has NO idea what a wall is. Moving the camera here never
// checks collision -- it just moves. Whoever calls Move/Strafe (the
// raycaster, or main loop) is expected to check the map first and only
// call these with a position it's already decided is valid.

typedef struct RCUT_Camera {
    float x, y;
    float dirX, dirY;     // facing direction, unit length
    float planeX, planeY; // camera plane, perpendicular to dir; length controls FOV
} RCUT_Camera;

// Sets position + facing angle (radians). Recomputes dir/plane from
// scratch -- planeLength controls FOV (0.66 =~ 66 degrees, a common
// raycaster default; smaller = narrower FOV).
RCUT_API void RCUT_Camera_Set(RCUT_Camera* cam, float x, float y, float angleRadians, float planeLength);

// Rotates dir/plane in place by the given angle (radians, positive =
// clockwise viewed from above). Does not touch position.
RCUT_API void RCUT_Camera_Rotate(RCUT_Camera* cam, float radians);

// Raw position moves -- no collision, no bounds checking. `amount` is in
// world/tile units, may be negative.
RCUT_API void RCUT_Camera_MoveForward(RCUT_Camera* cam, float amount); // along dir
RCUT_API void RCUT_Camera_Strafe(RCUT_Camera* cam, float amount);      // perpendicular to dir, +right

RCUT_API float RCUT_Camera_GetAngle(const RCUT_Camera* cam); // radians, derived from dir

// ---------------------------------------------------------------------
// Sprites
// ---------------------------------------------------------------------
// Billboards (NPCs, items, decorations) that always face the camera.
// Growable ID pool, same pattern as RCUT_Textures.

typedef int RCUT_SpriteId; // -1 = invalid

typedef struct RCUT_Sprite {
    float x, y;
    float scale;              // 1.0 = default size; see RCUT_Sprite_SetScale
    RCUT_TextureId textureId;
} RCUT_Sprite;

RCUT_API RCUT_SpriteId RCUT_Sprite_Add(float x, float y, RCUT_TextureId textureId);
RCUT_API void          RCUT_Sprite_Remove(RCUT_SpriteId id);
RCUT_API void          RCUT_Sprite_SetPos(RCUT_SpriteId id, float x, float y);
RCUT_API void          RCUT_Sprite_SetScale(RCUT_SpriteId id, float scale);

// Direct read access to a sprite's data -- returns NULL for an invalid/
// removed id. Transparent, like RCUT_Texture, since the raycaster will
// want to sort/iterate these every frame without per-field function
// call overhead.
RCUT_API const RCUT_Sprite* RCUT_Sprite_Get(RCUT_SpriteId id);

// For the raycaster's per-frame iteration: the raw backing array + how
// many slots are populated. NOT all slots in [0, count) are guaranteed
// alive -- check textureId >= 0 (removed slots are zeroed out) before
// using one.
RCUT_API const RCUT_Sprite* RCUT_Sprite_GetAll(int* outCount);

RCUT_API void RCUT_Sprite_RemoveAll(void);

#ifdef __cplusplus
}
#endif