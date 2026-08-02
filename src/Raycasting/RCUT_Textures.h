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

typedef int RCUT_TextureId; // -1 = invalid/failed load

typedef struct RCUT_Color {
    unsigned char r, g, b;
} RCUT_Color;

// A loaded texture's pixel data. Deliberately transparent (not opaque/
// handle-based) -- the raycaster reads straight out of `pixels` in its
// per-pixel sampling loop instead of paying a function-call cost per
// pixel, which matters at raycasting resolutions.
typedef struct RCUT_Texture {
    int width;
    int height;
    unsigned char* pixels; // RGBA, row-major, top-down, width*height*4 bytes
} RCUT_Texture;

// Loads an image (BMP/PNG/JPEG/TGA/...) from disk via stb_image.

// backgroundColor: if non-NULL, any pixel close to this color becomes
// fully transparent (alpha = 0) -- e.g. a sprite exported on a solid
// magenta background. Pass NULL to skip color-keying (e.g. a PNG that
// already has real per-pixel alpha, or an opaque wall/floor texture).

// alpha: overall opacity multiplier applied to every NON-keyed pixel,
// 0.0 = fully transparent, 1.0 = fully opaque. Use 1.0 for normal walls/
// sprites; lower it for things like tinted glass or a ghost enemy.

// Returns a texture id (>= 0), or -1 on failure (bad path/format).
RCUT_API RCUT_TextureId RCUT_Textures_Load(const char* path, const RCUT_Color* backgroundColor, float alpha);

// Looks up a loaded texture by id. Returns NULL for an invalid/unloaded id.
// The returned pointer is owned by RCUT_Textures -- don't free() it, and
// don't hold onto it across an Unload/UnloadAll call.
RCUT_API const RCUT_Texture* RCUT_Textures_Get(RCUT_TextureId id);

RCUT_API void RCUT_Textures_Unload(RCUT_TextureId id);
RCUT_API void RCUT_Textures_UnloadAll(void);

#ifdef __cplusplus
}
#endif