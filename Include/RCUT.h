#pragma once

#include <stddef.h>
#include <stdbool.h>

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

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------
// Window
// ---------------------------------------------------------------------

typedef struct RCUT_Window RCUT_Window;

typedef struct RCUT_WindowDesc {
    int width;
    int height;
    const char* title;
} RCUT_WindowDesc;

RCUT_API RCUT_Window* RCUT_Win_Create(const RCUT_WindowDesc* desc);
RCUT_API void RCUT_Win_Destroy(RCUT_Window* win);
RCUT_API bool RCUT_Win_PollEvents(RCUT_Window* win);
RCUT_API void RCUT_Win_Present(RCUT_Window* win, const unsigned char* rgbPixels, int width, int height);
RCUT_API int  RCUT_Win_GetWidth(const RCUT_Window* win);
RCUT_API int  RCUT_Win_GetHeight(const RCUT_Window* win);
RCUT_API bool RCUT_Win_IsOpen(const RCUT_Window* win);

// ---------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------

typedef enum RCUT_Key {
    RCUT_KEY_LEFT,
    RCUT_KEY_RIGHT,
    RCUT_KEY_UP,
    RCUT_KEY_DOWN,
    RCUT_KEY_COUNT
} RCUT_Key;

RCUT_API void RCUT_Input_Init(void);
RCUT_API void RCUT_Input_Shutdown(void);
RCUT_API void RCUT_Input_Update(void);
RCUT_API bool RCUT_Input_IsKeyDown(unsigned char key);
RCUT_API bool RCUT_Input_IsSpecialKeyDown(RCUT_Key key);
RCUT_API float RCUT_Input_GetMouseDeltaX(void);
RCUT_API float RCUT_Input_GetMouseDeltaY(void);

// ---------------------------------------------------------------------
// Textures
// ---------------------------------------------------------------------

typedef int RCUT_TextureId;

typedef struct RCUT_Color {
    unsigned char r, g, b;
} RCUT_Color;

typedef struct RCUT_Texture {
    int width;
    int height;
    unsigned char* pixels;
} RCUT_Texture;

RCUT_API RCUT_TextureId RCUT_Textures_Load(const char* path, const RCUT_Color* backgroundColor, float alpha);
RCUT_API const RCUT_Texture* RCUT_Textures_Get(RCUT_TextureId id);
RCUT_API void RCUT_Textures_Unload(RCUT_TextureId id);
RCUT_API void RCUT_Textures_UnloadAll(void);

// ---------------------------------------------------------------------
// Objects
// ---------------------------------------------------------------------

typedef struct RCUT_Camera {
    float x, y;
    float dirX, dirY;
    float planeX, planeY;
} RCUT_Camera;

RCUT_API void RCUT_Camera_Set(RCUT_Camera* cam, float x, float y, float angleRadians, float planeLength);
RCUT_API void RCUT_Camera_Rotate(RCUT_Camera* cam, float radians);
RCUT_API void RCUT_Camera_MoveForward(RCUT_Camera* cam, float amount);
RCUT_API void RCUT_Camera_Strafe(RCUT_Camera* cam, float amount);
RCUT_API float RCUT_Camera_GetAngle(const RCUT_Camera* cam);

typedef int RCUT_SpriteId;

typedef struct RCUT_Sprite {
    float x, y;
    float scale;
    RCUT_TextureId textureId;
} RCUT_Sprite;

RCUT_API RCUT_SpriteId RCUT_Sprite_Add(float x, float y, RCUT_TextureId textureId);
RCUT_API void          RCUT_Sprite_Remove(RCUT_SpriteId id);
RCUT_API void          RCUT_Sprite_SetPos(RCUT_SpriteId id, float x, float y);
RCUT_API void          RCUT_Sprite_SetScale(RCUT_SpriteId id, float scale);
RCUT_API const RCUT_Sprite* RCUT_Sprite_Get(RCUT_SpriteId id);
RCUT_API const RCUT_Sprite* RCUT_Sprite_GetAll(int* outCount);
RCUT_API void RCUT_Sprite_RemoveAll(void);

// ---------------------------------------------------------------------
// Raycaster
// ---------------------------------------------------------------------

typedef struct RCUT_MapDesc {
    const int* tiles;
    int width;
    int height;
} RCUT_MapDesc;

RCUT_API bool RCUT_Raycaster_Init(int screenWidth, int screenHeight);
RCUT_API void RCUT_Raycaster_Shutdown(void);
RCUT_API void RCUT_Raycaster_SetMap(const RCUT_MapDesc* desc);
RCUT_API void RCUT_Raycaster_SetWallTexture(int tileValue, RCUT_TextureId textureId);
RCUT_API void RCUT_Raycaster_SetTeleportTile(int tileValue);
RCUT_API void RCUT_Raycaster_SetFloorTexture(RCUT_TextureId textureId);
RCUT_API void RCUT_Raycaster_SetCeilingTexture(RCUT_TextureId textureId);
RCUT_API void RCUT_Raycaster_TryMove(RCUT_Camera* cam, float forwardAmount, float strafeAmount);
RCUT_API void RCUT_Raycaster_Render(const RCUT_Camera* cam);
RCUT_API const unsigned char* RCUT_Raycaster_GetFramebuffer(void);
RCUT_API int RCUT_Raycaster_GetWidth(void);
RCUT_API int RCUT_Raycaster_GetHeight(void);

// ---------------------------------------------------------------------
// Audio
// ---------------------------------------------------------------------

typedef int RCUT_SoundId;

RCUT_API bool RCUT_Audio_Init(void);
RCUT_API void RCUT_Audio_Shutdown(void);
RCUT_API RCUT_SoundId RCUT_Audio_LoadSFX(const char* path);
RCUT_API void RCUT_Audio_PlaySFX(RCUT_SoundId id, float volume);
RCUT_API bool RCUT_Music_Load(const char* path);
RCUT_API void RCUT_Music_Play(bool loop);
RCUT_API void RCUT_Music_Stop(void);
RCUT_API void RCUT_Music_Pause(void);
RCUT_API void RCUT_Music_Resume(void);
RCUT_API void RCUT_Music_SetVolume(float volume);
RCUT_API bool RCUT_Music_IsPlaying(void);

// ---------------------------------------------------------------------
// Engine
// ---------------------------------------------------------------------

typedef struct RCUT_EngineDesc {
    int width;
    int height;
    const char* title;
} RCUT_EngineDesc;

RCUT_API bool RCUT_Engine_Init(const RCUT_EngineDesc* desc);
RCUT_API void RCUT_Engine_Shutdown(void);
RCUT_API bool RCUT_Engine_BeginFrame(void);
RCUT_API void RCUT_Engine_EndFrame(void);
RCUT_API RCUT_Camera* RCUT_Engine_GetCamera(void);

#ifdef __cplusplus
}
#endif
