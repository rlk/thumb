# Thumb

Thumb is [my](http://kooima.net) personal real-time 3D graphics research codebase, presented in the form of a library. It provides a rendering engine with a highly-configurable shading system wrapping an ODE physical simulation with an in-engine world editor. A wide array of stereoscopic displays and virtual reality devices are supported, and a distributed synchronization mechanism enables cluster-parallel rendering.

## Dependencies

Thumb depends upon a number of external libraries. Binary distributions of Thumb provide these. However, if Thumb is to be built from its source then they must already be installed.

- [SDL2](http://www.libsdl.org) enables platform independence.
- [GLEW](http://glew.sourceforge.net) enables OpenGL portability.
- [FreeType](http://www.freetype.org) renders text.
- [libpng](http://www.libpng.org) reads PNG image files.
- [zlib](http://www.zlib.net) decompresses data.
- [ODE](http://ode.org) performs physics simulation.
- [xxd](http://grail.cba.csuohio.edu/%7Esomos/xxd-1.10.tar.gz) embeds data within the library.

The Thumb build uses a simple Makefile which seeks these dependencies using [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config/).