#include "RCUT_freeGlut_Bridge.h"

#include <GL/freeglut.h>
#include <windows.h>

static int  g_windowId = 0;   // 0 = no window exists
static int  g_width = 0;
static int  g_height = 0;
static bool g_closeRequested = false;
static bool g_glutInited = false;

// freeglut requires a display callback to be registered per window, even
// though we draw manually from RCUT_Bridge_SwapAndBlit instead of waiting
// for freeglut to call this back on its own schedule.
static void EmptyDisplayCallback(void) { }

static void ReshapeCallback(int w, int h)
{
    g_width = w;
    g_height = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h); // y-up, bottom-left origin
    glMatrixMode(GL_MODELVIEW);
}

static void CloseCallback(void)
{
    g_closeRequested = true;
}

bool RCUT_Bridge_CreateWindow(int width, int height, const char* title)
{
    if (g_windowId != 0) return false; // already have a window

    if (!g_glutInited) {
        int argc = 1;
        char* argv[1] = { (char*)"RCUT" };
        glutInit(&argc, argv);
        g_glutInited = true;
    }

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(width, height);
    g_windowId = glutCreateWindow(title ? title : "RCUT");
    if (g_windowId == 0) return false;

    g_width = width;
    g_height = height;
    g_closeRequested = false;

    glutDisplayFunc(EmptyDisplayCallback);
    glutReshapeFunc(ReshapeCallback);
    glutCloseFunc(CloseCallback);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    ReshapeCallback(width, height); // sets the initial ortho projection

    return true;
}

void RCUT_Bridge_DestroyWindow(void)
{
    if (g_windowId == 0) return;
    glutDestroyWindow(g_windowId);
    g_windowId = 0;
}

void RCUT_Bridge_PumpEvents(void)
{
    if (g_windowId == 0) return;
    glutMainLoopEvent(); // freeglut-only: one non-blocking event tick
}

bool RCUT_Bridge_ShouldClose(void)
{
    return g_closeRequested;
}

void RCUT_Bridge_SwapAndBlit(const unsigned char* rgbPixels, int width, int height)
{
    if (g_windowId == 0 || !rgbPixels) return;

    glutSetWindow(g_windowId);
    glClear(GL_COLOR_BUFFER_BIT);
    glRasterPos2i(0, 0);
    glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, rgbPixels);
    glutSwapBuffers();
}

int RCUT_Bridge_GetWidth(void)  { return g_width; }
int RCUT_Bridge_GetHeight(void) { return g_height; }

static void (*g_onKeyDown)(unsigned char) = NULL;
static void (*g_onKeyUp)(unsigned char) = NULL;
static void (*g_onSpecialDown)(RCUT_Bridge_SpecialKey) = NULL;
static void (*g_onSpecialUp)(RCUT_Bridge_SpecialKey) = NULL;
static void (*g_onMouseMove)(int, int) = NULL;

static RCUT_Bridge_SpecialKey TranslateGlutSpecialKey(int glutKey)
{
    switch (glutKey) {
        case GLUT_KEY_LEFT:  return RCUT_BRIDGE_KEY_LEFT;
        case GLUT_KEY_RIGHT: return RCUT_BRIDGE_KEY_RIGHT;
        case GLUT_KEY_UP:    return RCUT_BRIDGE_KEY_UP;
        case GLUT_KEY_DOWN:  return RCUT_BRIDGE_KEY_DOWN;
        default:              return RCUT_BRIDGE_KEY_UNKNOWN;
    }
}

// freeglut demands these exact signatures -- they just forward to
// whatever RCUT_IOStream (or anything else) registered above.
static void GlutKeyDownTrampoline(unsigned char key, int x, int y)      { (void)x; (void)y; if (g_onKeyDown) g_onKeyDown(key); }
static void GlutKeyUpTrampoline(unsigned char key, int x, int y)        { (void)x; (void)y; if (g_onKeyUp) g_onKeyUp(key); }
static void GlutSpecialDownTrampoline(int key, int x, int y)            { (void)x; (void)y; if (g_onSpecialDown) g_onSpecialDown(TranslateGlutSpecialKey(key)); }
static void GlutSpecialUpTrampoline(int key, int x, int y)              { (void)x; (void)y; if (g_onSpecialUp) g_onSpecialUp(TranslateGlutSpecialKey(key)); }
static void GlutPassiveMotionTrampoline(int x, int y)                   { if (g_onMouseMove) g_onMouseMove(x, y); }

void RCUT_Bridge_SetKeyCallbacks(void (*onKeyDown)(unsigned char), void (*onKeyUp)(unsigned char))
{
    g_onKeyDown = onKeyDown;
    g_onKeyUp = onKeyUp;
    glutKeyboardFunc(onKeyDown ? GlutKeyDownTrampoline : NULL);
    glutKeyboardUpFunc(onKeyUp ? GlutKeyUpTrampoline : NULL);
}

void RCUT_Bridge_SetSpecialKeyCallbacks(void (*onSpecialDown)(RCUT_Bridge_SpecialKey), void (*onSpecialUp)(RCUT_Bridge_SpecialKey))
{
    g_onSpecialDown = onSpecialDown;
    g_onSpecialUp = onSpecialUp;
    glutSpecialFunc(onSpecialDown ? GlutSpecialDownTrampoline : NULL);
    glutSpecialUpFunc(onSpecialUp ? GlutSpecialUpTrampoline : NULL);
}

void RCUT_Bridge_SetMouseMoveCallback(void (*onMouseMove)(int, int))
{
    g_onMouseMove = onMouseMove;
    glutPassiveMotionFunc(onMouseMove ? GlutPassiveMotionTrampoline : NULL);
}

void RCUT_Bridge_WarpPointer(int x, int y) { glutWarpPointer(x, y); }

void RCUT_Bridge_SetCursorVisible(bool visible)
{
    glutSetCursor(visible ? GLUT_CURSOR_LEFT_ARROW : GLUT_CURSOR_NONE);
}

void RCUT_Bridge_ConfineCursorToWindow(void)
{
    HWND hwnd = GetForegroundWindow();
    RECT rect;
    GetClientRect(hwnd, &rect);
    POINT topLeft = { rect.left, rect.top };
    POINT bottomRight = { rect.right, rect.bottom };
    ClientToScreen(hwnd, &topLeft);
    ClientToScreen(hwnd, &bottomRight);
    RECT screenRect = { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
    ClipCursor(&screenRect);
}