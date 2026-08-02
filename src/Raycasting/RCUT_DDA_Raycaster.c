#include "RCUT_DDA_Raycaster.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// ---------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------

static int g_screenW = 0, g_screenH = 0;
static unsigned char* g_framebuffer = NULL; // RGB, bottom-up
static float* g_zbuffer = NULL;             // per-column perpendicular wall distance

static int* g_map = NULL;
static int g_mapW = 0, g_mapH = 0;

typedef struct {
    int tileValue;
    RCUT_TextureId textureId;
    bool isTeleport;
} TileType;

static TileType* g_tileTypes = NULL;
static int g_tileTypeCount = 0;
static int g_tileTypeCapacity = 0;

static RCUT_TextureId g_floorTex = -1;
static RCUT_TextureId g_ceilTex = -1;

// ---------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------

static TileType* FindOrCreateTileType(int tileValue)
{
    for (int i = 0; i < g_tileTypeCount; i++) {
        if (g_tileTypes[i].tileValue == tileValue) return &g_tileTypes[i];
    }
    if (g_tileTypeCount == g_tileTypeCapacity) {
        g_tileTypeCapacity = g_tileTypeCapacity ? g_tileTypeCapacity * 2 : 8;
        g_tileTypes = (TileType*)realloc(g_tileTypes, sizeof(TileType) * g_tileTypeCapacity);
    }
    TileType* t = &g_tileTypes[g_tileTypeCount++];
    t->tileValue = tileValue;
    t->textureId = -1;
    t->isTeleport = false;
    return t;
}

static const TileType* FindTileType(int tileValue)
{
    for (int i = 0; i < g_tileTypeCount; i++) {
        if (g_tileTypes[i].tileValue == tileValue) return &g_tileTypes[i];
    }
    return NULL;
}

// Returns the raw tile value, or -1 if (x,y) is outside the map.
static int GetTile(int x, int y)
{
    if (x < 0 || x >= g_mapW || y < 0 || y >= g_mapH) return -1;
    return g_map[y * g_mapW + x];
}

// yFromTop==0 is the TOP of the screen (intuitive for wall/floor loops);
// this flips into the bottom-up buffer RCUT_Win_Present expects.
static void PutPixel(int x, int yFromTop, unsigned char r, unsigned char g, unsigned char b)
{
    if (x < 0 || x >= g_screenW || yFromTop < 0 || yFromTop >= g_screenH) return;
    int row = g_screenH - 1 - yFromTop;
    unsigned char* p = &g_framebuffer[(row * g_screenW + x) * 3];
    p[0] = r; p[1] = g; p[2] = b;
}

static void SampleTexture(const RCUT_Texture* tex, int tx, int ty,
                           unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a)
{
    if (tx < 0) {
        tx = 0;
    }
    if (tx >= tex->width) {
        tx = tex->width - 1;
    }
    if (ty < 0) {
        ty = 0;
    }
    if (ty >= tex->height) {
        ty = tex->height - 1;
    }
    const unsigned char* p = &tex->pixels[(ty * tex->width + tx) * 4];
    *r = p[0]; *g = p[1]; *b = p[2]; *a = p[3];
}

// ---------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------

bool RCUT_Raycaster_Init(int screenWidth, int screenHeight)
{
    if (screenWidth <= 0 || screenHeight <= 0) return false;
    g_screenW = screenWidth;
    g_screenH = screenHeight;
    g_framebuffer = (unsigned char*)malloc((size_t)g_screenW * g_screenH * 3);
    g_zbuffer = (float*)malloc(sizeof(float) * g_screenW);
    return g_framebuffer && g_zbuffer;
}

void RCUT_Raycaster_Shutdown(void)
{
    free(g_framebuffer); g_framebuffer = NULL;
    free(g_zbuffer);     g_zbuffer = NULL;
    free(g_map);         g_map = NULL;
    free(g_tileTypes);   g_tileTypes = NULL;
    g_tileTypeCount = g_tileTypeCapacity = 0;
    g_screenW = g_screenH = g_mapW = g_mapH = 0;
    g_floorTex = g_ceilTex = -1;
}

// ---------------------------------------------------------------------
// Map
// ---------------------------------------------------------------------

void RCUT_Raycaster_SetMap(const RCUT_MapDesc* desc)
{
    if (!desc || !desc->tiles || desc->width <= 0 || desc->height <= 0) return;
    g_mapW = desc->width;
    g_mapH = desc->height;
    free(g_map);
    size_t n = (size_t)g_mapW * g_mapH;
    g_map = (int*)malloc(sizeof(int) * n);
    memcpy(g_map, desc->tiles, sizeof(int) * n);
}

void RCUT_Raycaster_SetWallTexture(int tileValue, RCUT_TextureId textureId)
{
    FindOrCreateTileType(tileValue)->textureId = textureId;
}

void RCUT_Raycaster_SetTeleportTile(int tileValue)
{
    FindOrCreateTileType(tileValue)->isTeleport = true;
}

void RCUT_Raycaster_SetFloorTexture(RCUT_TextureId textureId)   { g_floorTex = textureId; }
void RCUT_Raycaster_SetCeilingTexture(RCUT_TextureId textureId) { g_ceilTex = textureId; }

// ---------------------------------------------------------------------
// Collision-checked movement
// ---------------------------------------------------------------------

// Tries to move `*coord` (the axis being moved) by `delta`, given the
// other axis's current value. Blocks on walls, wraps on teleport tiles.
static void TryMoveAxis(float* coord, float other, float delta, bool xAxis)
{
    float target = *coord + delta;
    int tx = xAxis ? (int)target : (int)other;
    int ty = xAxis ? (int)other  : (int)target;

    int tile = GetTile(tx, ty);
    if (tile == 0) {
        *coord = target;
        return;
    }
    if (tile > 0) {
        const TileType* t = FindTileType(tile);
        if (t && t->isTeleport) {
            if (xAxis) *coord = (tx <= 0) ? (float)(g_mapW - 2) + 0.5f : 1.5f;
            else       *coord = (ty <= 0) ? (float)(g_mapH - 2) + 0.5f : 1.5f;
            return;
        }
    }
    // else: solid wall or out of bounds -- blocked, leave *coord unchanged
}

void RCUT_Raycaster_TryMove(RCUT_Camera* cam, float forwardAmount, float strafeAmount)
{
    if (!cam) return;

    float dx = cam->dirX * forwardAmount + cam->dirY * strafeAmount;
    float dy = cam->dirY * forwardAmount + (-cam->dirX) * strafeAmount;

    TryMoveAxis(&cam->x, cam->y, dx, true);
    TryMoveAxis(&cam->y, cam->x, dy, false);
}

// ---------------------------------------------------------------------
// Render: floor + ceiling
// ---------------------------------------------------------------------

static void RenderFloorAndCeiling(const RCUT_Camera* cam)
{
    int halfH = g_screenH / 2;

    float rayDirX0 = cam->dirX - cam->planeX, rayDirY0 = cam->dirY - cam->planeY;
    float rayDirX1 = cam->dirX + cam->planeX, rayDirY1 = cam->dirY + cam->planeY;

    for (int y = 0; y < g_screenH; y++) {
        bool isFloor = y >= halfH;
        int p = isFloor ? (y - halfH) : (halfH - y);
        if (p == 0) p = 1; // avoid divide-by-zero on the horizon line

        float rowDistance = (0.5f * g_screenH) / (float)p;

        float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / g_screenW;
        float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / g_screenW;

        float floorX = cam->x + rowDistance * rayDirX0;
        float floorY = cam->y + rowDistance * rayDirY0;

        RCUT_TextureId texId = isFloor ? g_floorTex : g_ceilTex;
        const RCUT_Texture* tex = (texId >= 0) ? RCUT_Textures_Get(texId) : NULL;

        for (int x = 0; x < g_screenW; x++) {
            unsigned char r, g, b;

            if (tex) {
                float fracX = floorX - floorf(floorX);
                float fracY = floorY - floorf(floorY);
                int tx = (int)(fracX * tex->width);
                int ty = (int)(fracY * tex->height);
                unsigned char a;
                SampleTexture(tex, tx, ty, &r, &g, &b, &a);
            } else {
                unsigned char shade = isFloor ? 60 : 40;
                r = g = b = shade;
            }

            PutPixel(x, y, r, g, b);

            floorX += floorStepX;
            floorY += floorStepY;
        }
    }
}

// ---------------------------------------------------------------------
// Render: walls (DDA)
// ---------------------------------------------------------------------

static void RenderWalls(const RCUT_Camera* cam)
{
    for (int x = 0; x < g_screenW; x++) {
        float cameraX = 2.0f * x / (float)g_screenW - 1.0f;
        float rayDirX = cam->dirX + cam->planeX * cameraX;
        float rayDirY = cam->dirY + cam->planeY * cameraX;

        int mapX = (int)cam->x, mapY = (int)cam->y;

        float deltaDistX = (rayDirX == 0) ? 1e30f : fabsf(1.0f / rayDirX);
        float deltaDistY = (rayDirY == 0) ? 1e30f : fabsf(1.0f / rayDirY);

        int stepX, stepY;
        float sideDistX, sideDistY;

        if (rayDirX < 0) { stepX = -1; sideDistX = (cam->x - mapX) * deltaDistX; }
        else             { stepX = 1;  sideDistX = (mapX + 1.0f - cam->x) * deltaDistX; }

        if (rayDirY < 0) { stepY = -1; sideDistY = (cam->y - mapY) * deltaDistY; }
        else             { stepY = 1;  sideDistY = (mapY + 1.0f - cam->y) * deltaDistY; }

        int side = 0, tileVal = 0;
        bool hit = false;

        while (!hit) {
            if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; side = 0; }
            else                       { sideDistY += deltaDistY; mapY += stepY; side = 1; }

            int tile = GetTile(mapX, mapY);
            if (tile < 0) { tileVal = 1; hit = true; break; } // out-of-bounds acts as a wall
            if (tile != 0) { tileVal = tile; hit = true; }
        }

        float perpWallDist = (side == 0) ? (sideDistX - deltaDistX) : (sideDistY - deltaDistY);
        if (perpWallDist < 0.0001f) perpWallDist = 0.0001f;
        g_zbuffer[x] = perpWallDist;

        int lineHeight = (int)(g_screenH / perpWallDist);
        int drawStart = -lineHeight / 2 + g_screenH / 2;
        int drawEnd   =  lineHeight / 2 + g_screenH / 2;

        float wallX = (side == 0) ? (cam->y + perpWallDist * rayDirY)
                                   : (cam->x + perpWallDist * rayDirX);
        wallX -= floorf(wallX);

        const TileType* tt = FindTileType(tileVal);
        RCUT_TextureId texId = tt ? tt->textureId : -1;
        const RCUT_Texture* tex = (texId >= 0) ? RCUT_Textures_Get(texId) : NULL;

        int yStart = drawStart < 0 ? 0 : drawStart;
        int yEnd   = drawEnd >= g_screenH ? g_screenH - 1 : drawEnd;

        if (tex) {
            int texX = (int)(wallX * tex->width);
            if (side == 0 && rayDirX > 0) texX = tex->width - texX - 1;
            if (side == 1 && rayDirY < 0) texX = tex->width - texX - 1;
            if (texX < 0) {
                texX = 0;
            }
            if (texX >= tex->width) {
                texX = tex->width - 1;
            }

            float shade = (side == 1) ? 0.7f : 1.0f;

            for (int y = yStart; y <= yEnd; y++) {
                int texY = ((y * 2 - g_screenH + lineHeight) * tex->height) / (2 * lineHeight);
                unsigned char r, g, b, a;
                SampleTexture(tex, texX, texY, &r, &g, &b, &a);
                PutPixel(x, y, (unsigned char)(r * shade), (unsigned char)(g * shade), (unsigned char)(b * shade));
            }
        } else {
            unsigned char r, g, b;
            bool teleport = tt && tt->isTeleport;
            if (teleport)       { r = 0;   g = 0; b = 255; }
            else if (side == 0) { r = 255; g = 0; b = 0;   }
            else                { r = 180; g = 0; b = 0;   }

            for (int y = yStart; y <= yEnd; y++) PutPixel(x, y, r, g, b);
        }
    }
}

// ---------------------------------------------------------------------
// Render: sprites (billboards)
// ---------------------------------------------------------------------

static const RCUT_Camera* g_sortCam; // for the qsort comparator below

static int CompareSpriteFarthestFirst(const void* a, const void* b)
{
    const RCUT_Sprite* sa = *(const RCUT_Sprite* const*)a;
    const RCUT_Sprite* sb = *(const RCUT_Sprite* const*)b;
    float da = (sa->x - g_sortCam->x) * (sa->x - g_sortCam->x) + (sa->y - g_sortCam->y) * (sa->y - g_sortCam->y);
    float db = (sb->x - g_sortCam->x) * (sb->x - g_sortCam->x) + (sb->y - g_sortCam->y) * (sb->y - g_sortCam->y);
    if (da > db) return -1; // farthest first
    if (da < db) return 1;
    return 0;
}

static void RenderSprites(const RCUT_Camera* cam)
{
    int spriteCount = 0;
    const RCUT_Sprite* allSprites = RCUT_Sprite_GetAll(&spriteCount);
    if (spriteCount == 0) return;

    // Build a list of pointers to the LIVE sprites only, then sort those.
    const RCUT_Sprite** order = (const RCUT_Sprite**)malloc(sizeof(RCUT_Sprite*) * spriteCount);
    int liveCount = 0;
    for (int i = 0; i < spriteCount; i++) {
        if (allSprites[i].textureId >= 0) order[liveCount++] = &allSprites[i];
    }

    g_sortCam = cam;
    qsort(order, liveCount, sizeof(RCUT_Sprite*), CompareSpriteFarthestFirst);

    float invDet = 1.0f / (cam->planeX * cam->dirY - cam->dirX * cam->planeY);

    for (int i = 0; i < liveCount; i++) {
        const RCUT_Sprite* spr = order[i];
        const RCUT_Texture* tex = RCUT_Textures_Get(spr->textureId);
        if (!tex) continue;

        float sx = spr->x - cam->x;
        float sy = spr->y - cam->y;

        float transformX = invDet * (cam->dirY * sx - cam->dirX * sy);
        float transformY = invDet * (-cam->planeY * sx + cam->planeX * sy); // depth

        if (transformY <= 0.01f) continue; // behind camera

        int spriteScreenX = (int)((g_screenW / 2) * (1.0f + transformX / transformY));

        int spriteHeight = (int)fabsf(g_screenH / transformY * spr->scale);
        int drawStartY = -spriteHeight / 2 + g_screenH / 2;
        if (drawStartY < 0) {
            drawStartY = 0;
        }
        int drawEndY   =  spriteHeight / 2 + g_screenH / 2;
        if (drawEndY >= g_screenH) {
            drawEndY = g_screenH - 1;
        }

        int spriteWidth = (int)fabsf(g_screenH / transformY * spr->scale); // square billboard
        int spriteLeft = -spriteWidth / 2 + spriteScreenX;
        int drawStartX = spriteLeft;
        if (drawStartX < 0) {
            drawStartX = 0;
        }
        int drawEndX   = spriteWidth / 2 + spriteScreenX;
        if (drawEndX >= g_screenW) {
            drawEndX = g_screenW - 1;
        }

        for (int stripe = drawStartX; stripe <= drawEndX; stripe++) {
            if (transformY >= g_zbuffer[stripe]) continue; // occluded by a wall

            int texX = (int)((stripe - spriteLeft) * tex->width / (float)spriteWidth);
            if (texX < 0) {
                texX = 0;
            }
            if (texX >= tex->width) {
                texX = tex->width - 1;
            }

            for (int y = drawStartY; y <= drawEndY; y++) {
                int d = (y - g_screenH / 2) + spriteHeight / 2;
                int texY = (int)(d * tex->height / (float)spriteHeight);
                if (texY < 0) {
                    texY = 0;
                }
                if (texY >= tex->height) {
                    texY = tex->height - 1;
                }

                unsigned char r, g, b, a;
                SampleTexture(tex, texX, texY, &r, &g, &b, &a);
                if (a < 128) continue; // transparent (or below alpha threshold) pixel

                PutPixel(stripe, y, r, g, b);
            }
        }
    }

    free(order);
}

// ---------------------------------------------------------------------

void RCUT_Raycaster_Render(const RCUT_Camera* cam)
{
    if (!cam || g_screenW == 0 || g_screenH == 0) return;
    RenderFloorAndCeiling(cam);
    RenderWalls(cam);
    RenderSprites(cam);
}

const unsigned char* RCUT_Raycaster_GetFramebuffer(void) { return g_framebuffer; }
int RCUT_Raycaster_GetWidth(void)  { return g_screenW; }
int RCUT_Raycaster_GetHeight(void) { return g_screenH; }