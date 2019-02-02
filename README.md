# Glyph-Packing-Example
This project is a testing ground for glyph packing from freetype generated character bitmaps.
This project is not meant to be anything formal and has only been setup for OSX.

**```Note:```** This project currently does not work because the included freetype library has not been recompiled for newer MacOS versions.

## Dependencies:
These are the libraries used by this project:

- GLFW library
- Freetype library

They have already been compiled and are included in the project. 
They are dynamically linked via @rpath runtime linking.

## Build Process
The following two scripts are for building & running the project:

- **```./build.sh```** will build the executable.
- **```./run.sh```** will build and then run the executable.

## Running
The process must be run from the projects root directory.
E.g. ```./build/main```
