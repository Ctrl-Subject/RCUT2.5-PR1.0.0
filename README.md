# RCUT 2.5D Archive

This repository serves as the historical archive for the development of the **RCUT (Rendering Core Utility Toolkit)** 2.5D DDA rendering engine. It contains the complete progression of the project, from its earliest experimental prototypes through to some of the first fully featured 2.5D versions of the engine.

RCUT was originally developed as the rendering engine for my **A-Level OCR Computer Science NEA (Non-Exam Assessment)**. The aim was to create a custom 2.5D DDA raycasting engine rather than relying on an existing game engine or rendering framework. As development progressed, the project expanded far beyond its original educational purpose, becoming a standalone rendering toolkit and an important foundation for future graphics technologies.

The purpose of this archive is to preserve that development history, allowing others to explore how the engine evolved over time. Each revision demonstrates different stages of the project's design, optimisation, rendering techniques, and feature implementation. This repository should therefore be viewed as a historical record of RCUT's development rather than the actively maintained version of the engine.

RCUT is a product of the **Solar Project**, developed by **GP Software**. Many of the ideas, techniques, and technologies first explored in RCUT have since influenced the wider Solar ecosystem and continue to be developed as part of that project.

Development of RCUT continued beyond the versions contained in this archive. For newer releases and ongoing development (January 2026 onwards), please visit the GP Software GitHub organisation or the official GP Software website (once available).

## License

RCUT is released under the **MIT License**. You are free to use, modify, copy, merge, publish, distribute, sublicense, and/or sell copies of the software, provided that the copyright notice and permission notice are included in all copies or substantial portions of the software.

For the full license terms, please refer to the `LICENSE` file included in this repository.

## Compiler Command

This was originally compiled using the mingw g++ 64-bit compiler
The command used is:

g++ src/main.cpp -o RCUT.exe -IInclude -LLibraries -lfreeglut -lopengl32 -lglu32 -lgdi32 -lwinmm
