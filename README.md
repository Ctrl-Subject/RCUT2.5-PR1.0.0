# RCUT 2.5D Archive

This repository contains the archive of the **RCUT (Rendering Core Utility Toolkit)** 2.5D DDA raycasting engine. It is organised as a historic snapshot of the project, with the source tree, external dependencies, and a minimal build path for Windows.

## Contents

- `Include/` – public RCUT API header
- `src/` – engine implementation and example sources
- `src/Application/` – window, input, audio, and platform bridge code
- `src/IOStream/` – input handling glue
- `src/Raycasting/` – raycaster, textures, sprites, and map logic
- `src/Extern/` – bundled external dependencies and platform headers

## Build Overview

This archive is intended to build as a DLL on Windows.

## CMake Usage

A `CMakeLists.txt` file is available at the repository root.
To generate a DLL build with CMake, run:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build

cmake -S . -B build_dll -G "MinGW Makefiles" -DRCUT_BUILD_SHARED=ON
cmake --build build_dll
```

The output will include `RCUT.dll` and the import library `libRCUT.a`.

## Dependencies

- `freeglut` library and headers (bundled under `src/Extern/Includes/GL` and `src/Extern/Libraries`)
- `OpenGL` system libraries: `opengl32`, `glu32`, `gdi32`, `winmm`
- `stb_image.h` for texture loading
- `miniaudio.h` for audio playback

## Recommended workflow

1. Open a shell in the repository root.
2. Install MSYS2/MinGW if needed.
3. Run the build command above.
4. Use `RCUT.dll` and `libRCUT.a` from the output directory.

## Project Notes

This archive is primarily historic. It documents an engine built as part of a UK A-Level NEA project and preserved as a reference implementation rather than a production library.

## License

RCUT is released under the **MIT License**. See the `LICENSE` file for the full terms.
