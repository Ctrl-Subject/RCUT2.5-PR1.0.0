# RCUT IOStream Module

This module handles input state and converts low-level window/key events into RCUT-friendly key queries.

## Files

- `RCUT_IOSTREAM.c` – implementation of keyboard and special-key tracking.
- `RCUT_IOSTREAM.h` – public API for querying key state.

## What it provides

- `RCUT_Input_Init()` / `RCUT_Input_Shutdown()` – start and stop the input system.
- `RCUT_Input_Update()` – refreshes frame-by-frame input state.
- `RCUT_Input_IsKeyDown(unsigned char key)` – checks raw ASCII keys.
- `RCUT_Input_IsSpecialKeyDown(RCUT_Key key)` – checks directional/special keys.

## Integration

This module depends on the `RCUT_freeGlut_Bridge` internal interface to receive events from FreeGLUT and translate them into usable game input.
