# RCUT Application Module

This module contains the platform-specific application support for RCUT.

## Files

- `RCUT_audioSauce.c`, `RCUT_audioSauce.h` – audio playback using `miniaudio`.
- `RCUT_freeGlut_Bridge.c`, `RCUT_freeGlut_Bridge.h` – internal freeglut wrapper used by the window/input bridge.
- `RCUT_Win.c`, `RCUT_Win.h` – window creation, event pumping, and framebuffer presentation.
- `miniaudio.h` – single-file miniaudio implementation included in the source tree.

## Responsibilities

- Create and destroy the rendering window.
- Pump OS and input events via FreeGLUT.
- Present the engine's framebuffer to the window.
- Manage audio initialization, sound loading, and music playback.

## Notes

- `RCUT_freeGlut_Bridge.h` is intentionally internal: it is not part of the public `Include/RCUT.h` API.
- `miniaudio.h` is compiled in only once via `RCUT_audioSauce.c` using `#define MINIAUDIO_IMPLEMENTATION`.
