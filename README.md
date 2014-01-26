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

## File system

When distributing an application, it's convenient to distribute supporting assets (images, models, shaders, etc.) in a compact archival form. It's critical to ensure an archive is internally self-consistent and does not refer to assets through absolute references to the developer's file system. To support this, Thumb implements a virtual filesystem mechanism.

The virtual filesystem stores assets with relative path names, overlaying a list of data sources that may include user directories, ZIP archives, and statically-linked binaries. This overlay allows user-defined assets to override application assets freely. Of course, normal access to the root file system remains available along side the virtual file system.

File selection dialogs display the current directory in a text edit box at the top. The root file system is accessed for all absolute paths (directories begining with `/` or `C:/`), and the virtual file system is used for all relative paths. Thus, to view the root filesystem, simply change the directory to `/` and press enter. To view the virtual file system, clear the directory text edit box.
