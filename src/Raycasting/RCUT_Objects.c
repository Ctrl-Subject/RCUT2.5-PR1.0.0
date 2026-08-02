#include "RCUT_Objects.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------

void RCUT_Camera_Set(RCUT_Camera* cam, float x, float y, float angleRadians, float planeLength)
{
    if (!cam) return;
    cam->x = x;
    cam->y = y;
    cam->dirX = cosf(angleRadians);
    cam->dirY = sinf(angleRadians);
    cam->planeX = -cam->dirY * planeLength;
    cam->planeY =  cam->dirX * planeLength;
}

void RCUT_Camera_Rotate(RCUT_Camera* cam, float radians)
{
    if (!cam) return;
    float c = cosf(radians), s = sinf(radians);

    float oldDirX = cam->dirX;
    cam->dirX = cam->dirX * c - cam->dirY * s;
    cam->dirY = oldDirX * s + cam->dirY * c;

    float oldPlaneX = cam->planeX;
    cam->planeX = cam->planeX * c - cam->planeY * s;
    cam->planeY = oldPlaneX * s + cam->planeY * c;
}

void RCUT_Camera_MoveForward(RCUT_Camera* cam, float amount)
{
    if (!cam) return;
    cam->x += cam->dirX * amount;
    cam->y += cam->dirY * amount;
}

void RCUT_Camera_Strafe(RCUT_Camera* cam, float amount)
{
    if (!cam) return;
    // Perpendicular to facing; matches dir rotated -90 degrees.
    cam->x += cam->dirY * amount;
    cam->y += -cam->dirX * amount;
}

float RCUT_Camera_GetAngle(const RCUT_Camera* cam)
{
    if (!cam) return 0.0f;
    return atan2f(cam->dirY, cam->dirX);
}

// ---------------------------------------------------------------------
// Sprites
// ---------------------------------------------------------------------

static RCUT_Sprite* g_sprites = NULL;
static int g_spriteCount = 0;    // number of slots ever used (high-water mark)
static int g_spriteCapacity = 0;

static int FindFreeSlot(void)
{
    for (int i = 0; i < g_spriteCount; i++) {
        if (g_sprites[i].textureId < 0) return i; // textureId < 0 marks a dead slot
    }
    if (g_spriteCount == g_spriteCapacity) {
        g_spriteCapacity = g_spriteCapacity ? g_spriteCapacity * 2 : 16;
        g_sprites = (RCUT_Sprite*)realloc(g_sprites, sizeof(RCUT_Sprite) * g_spriteCapacity);
    }
    return g_spriteCount++;
}

RCUT_SpriteId RCUT_Sprite_Add(float x, float y, RCUT_TextureId textureId)
{
    if (textureId < 0) return -1;

    int slot = FindFreeSlot();
    g_sprites[slot].x = x;
    g_sprites[slot].y = y;
    g_sprites[slot].scale = 1.0f;
    g_sprites[slot].textureId = textureId;

    return (RCUT_SpriteId)slot;
}

void RCUT_Sprite_Remove(RCUT_SpriteId id)
{
    if (id < 0 || id >= g_spriteCount) return;
    memset(&g_sprites[id], 0, sizeof(RCUT_Sprite));
    g_sprites[id].textureId = -1; // dead-slot marker
}

void RCUT_Sprite_SetPos(RCUT_SpriteId id, float x, float y)
{
    if (id < 0 || id >= g_spriteCount || g_sprites[id].textureId < 0) return;
    g_sprites[id].x = x;
    g_sprites[id].y = y;
}

void RCUT_Sprite_SetScale(RCUT_SpriteId id, float scale)
{
    if (id < 0 || id >= g_spriteCount || g_sprites[id].textureId < 0) return;
    g_sprites[id].scale = scale;
}

const RCUT_Sprite* RCUT_Sprite_Get(RCUT_SpriteId id)
{
    if (id < 0 || id >= g_spriteCount || g_sprites[id].textureId < 0) return NULL;
    return &g_sprites[id];
}

const RCUT_Sprite* RCUT_Sprite_GetAll(int* outCount)
{
    if (outCount) *outCount = g_spriteCount;
    return g_sprites;
}

void RCUT_Sprite_RemoveAll(void)
{
    free(g_sprites);
    g_sprites = NULL;
    g_spriteCount = 0;
    g_spriteCapacity = 0;
}