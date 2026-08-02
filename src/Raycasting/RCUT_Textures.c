#include "RCUT_Textures.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdlib.h>
#include <string.h>

// How close an RGB value has to be to backgroundColor to get keyed out.
// A little slack (rather than an exact match) keeps anti-aliased edges
// around the key color from leaving a visible fringe.
#define RCUT_COLOR_KEY_TOLERANCE 10

typedef struct {
    RCUT_Texture texture;
    bool inUse;
} Slot;

static Slot* g_slots = NULL;
static int   g_slotCount = 0;
static int   g_slotCapacity = 0;

static bool ColorClose(unsigned char r, unsigned char g, unsigned char b, RCUT_Color key)
{
    int dr = (int)r - (int)key.r;
    int dg = (int)g - (int)key.g;
    int db = (int)b - (int)key.b;
    return (dr * dr + dg * dg + db * db) <= (RCUT_COLOR_KEY_TOLERANCE * RCUT_COLOR_KEY_TOLERANCE * 3);
}

static int FindFreeSlot(void)
{
    for (int i = 0; i < g_slotCount; i++) {
        if (!g_slots[i].inUse) return i;
    }
    if (g_slotCount == g_slotCapacity) {
        g_slotCapacity = g_slotCapacity ? g_slotCapacity * 2 : 8;
        g_slots = (Slot*)realloc(g_slots, sizeof(Slot) * g_slotCapacity);
    }
    return g_slotCount++;
}

RCUT_TextureId RCUT_Textures_Load(const char* path, const RCUT_Color* backgroundColor, float alpha)
{
    int w, h, srcChannels;
    unsigned char* data = stbi_load(path, &w, &h, &srcChannels, 4); // always force RGBA
    if (!data) return -1;

    unsigned char clampedAlpha = (unsigned char)(alpha < 0.0f ? 0 : (alpha > 1.0f ? 255 : alpha * 255.0f));

    for (int i = 0; i < w * h; i++) {
        unsigned char* px = &data[i * 4];
        if (backgroundColor && ColorClose(px[0], px[1], px[2], *backgroundColor)) {
            px[3] = 0; // keyed out -- fully transparent regardless of `alpha`
        } else {
            // Scale existing alpha (from a real PNG alpha channel, or 255
            // if the source had none) by the requested opacity multiplier.
            px[3] = (unsigned char)(((int)px[3] * (int)clampedAlpha) / 255);
        }
    }

    int slot = FindFreeSlot();
    g_slots[slot].texture.width = w;
    g_slots[slot].texture.height = h;
    g_slots[slot].texture.pixels = data; // stb_image-allocated, freed via stbi_image_free
    g_slots[slot].inUse = true;

    return (RCUT_TextureId)slot;
}

const RCUT_Texture* RCUT_Textures_Get(RCUT_TextureId id)
{
    if (id < 0 || id >= g_slotCount || !g_slots[id].inUse) return NULL;
    return &g_slots[id].texture;
}

void RCUT_Textures_Unload(RCUT_TextureId id)
{
    if (id < 0 || id >= g_slotCount || !g_slots[id].inUse) return;
    stbi_image_free(g_slots[id].texture.pixels);
    memset(&g_slots[id].texture, 0, sizeof(RCUT_Texture));
    g_slots[id].inUse = false;
}

void RCUT_Textures_UnloadAll(void)
{
    for (int i = 0; i < g_slotCount; i++) {
        if (g_slots[i].inUse) {
            stbi_image_free(g_slots[i].texture.pixels);
        }
    }
    free(g_slots);
    g_slots = NULL;
    g_slotCount = 0;
    g_slotCapacity = 0;
}