# RCUT Raycasting Module

This module contains the raycasting engine, texture loading, sprite rendering, and map management.

## Files

- `RCUT_DDA_Raycaster.c`, `RCUT_DDA_Raycaster.h` – core raycasting, map handling, and rendering.
- `RCUT_Objects.c`, `RCUT_Objects.h` – camera control, sprite management, and object APIs.
- `RCUT_Textures.c`, `RCUT_Textures.h` – texture loading using `stb_image` and texture lifetime management.
- `stb_image.h` – bundled image loader for PNG/JPG and other texture formats.

## Responsibilities

- Load texture images from disk.
- Manage texture slots and sprite storage.
- Render walls, floors, ceilings, and sprites with DDA raycasting.
- Provide collision-aware camera movement and teleport tile support.

## Notes

- Textures are loaded into RGBA format and may be keyed transparent based on a background color.
- Sprites are rendered as billboards and sorted from farthest to nearest each frame.
