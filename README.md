# Glyph-Packing
This project is a testing ground for glyph packing from freetype generated character bitmaps.
This project is not meant to be anything formal and has only been setup for OSX.

## Dependencies:
### OSX:
These are the libraries used by this project:

- GLFW library
- Freetype library

They have already been compiled and are included in the project. 
They are dynamically linked via @rpath runtime linking.

## Build Process
### OSX:
The following are the two script relating to building & running the project:

- **./build.sh** will build the executable.
- **./run.sh** will build and then run the executable.

## TODO:
- FIX: The bitmap alignment issue has been fixed but the work around code has not been removed yet. Cleanup is required here.