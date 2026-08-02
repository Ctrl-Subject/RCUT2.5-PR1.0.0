# RCUT Externals

This folder contains external libraries and headers used by RCUT.

## Contents

- `Includes/GL/` – FreeGLUT/OpenGL-compatible headers used by the sample application.
- `Libraries/` – static FreeGLUT libraries used for Windows linking.

## Notes

- The project builds against the bundled `freeglut` headers and library files by default.
- If you replace these with system-installed versions, update the include and library paths in the build command accordingly.
