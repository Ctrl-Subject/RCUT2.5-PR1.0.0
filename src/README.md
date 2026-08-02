# RCUT Source Tree

This folder contains the implementation sources for the RCUT engine.

## Subdirectories

- `Application/` – window management, input handling, audio, and platform bridge code.
- `IOStream/` – input system implementation and key mapping.
- `Raycasting/` – raycasting engine, texture loading, sprite support, and map logic.
- `Extern/` – bundled third-party headers and libraries used by the project.

## How this folder is used

The source files here are compiled together to produce the RCUT library. The public engine API is exposed through `Include/RCUT.h`, and the implementation is split into module-specific files for organisation and maintainability.
