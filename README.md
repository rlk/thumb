# Thumb

Thumb is [my](http://kooima.net) real-time 3D graphics research codebase, presented in the form of a library. It provides a rendering engine with a highly-configurable shading system wrapping an ODE physical simulation with an in-engine world editor. A wide array of stereoscopic displays and virtual reality devices are supported, and a distributed synchronization mechanism enables cluster-parallel rendering.

## Dependencies

Thumb depends upon a number of external libraries. Binary distributions of Thumb provide these. However, if Thumb is to be built from its source then they must already be installed.

- [SDL2](http://www.libsdl.org) enables platform independence.
- [GLEW](http://glew.sourceforge.net) enables OpenGL portability.
- [FreeType](http://www.freetype.org) renders text.
- [libpng](http://www.libpng.org) reads PNG image files.
- [zlib](http://www.zlib.net) decompresses data.
- [ODE](http://ode.org) performs physics simulation.
- [zip](http://www.info-zip.org/Zip.html) helps embed static data.

Optionally,

- [LibOVR](https://developer.oculus.com) enables Oculus Rift support.

## Build

### Linux and OS X

Linux and OS X builds are driven by Makefiles. Dependencies are managed by [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config/). Simply run `make` to produce `Release/libthumb.a`:

	make

To produce a debug build of the library `Debug/libthumb.a`:

	make DEBUG=1

To produce a dynamically linkable library:

	make DYNAMIC=1

### Windows

The Windows build is driven by `nmake` files named `Makefile.vc`. These include some local configuration that *must* be set.

- `LOCAL_INCLUDE` gives the location of the dependency headers. Default: `C:\Include`

- `LIBOVR` gives the root of the `LibOVR` directory in the Oculus SDK. Default: `C:\OculusSDK\LibOVR`

- `ZIP` gives the path and options for the ZIP command. Default: `C:\Bin\zip.exe -9`

The build is activated using either the included Visual Studio project file or the developer's command prompt. On the command prompt, use `nmake` to produce `Release/thumb.lib`:

	nmake /f Makefile.vc

To produce a debug build of the library `Debug/thumb.lib`

	nmake /f Makefile.vc DEBUG=1

The current Windows build does not support dynamic linking.

## File system

When distributing an application, it's convenient to distribute supporting assets (images, models, shaders, etc.) in a compact archival form. It's critical to ensure an archive is internally self-consistent and does not refer to assets through absolute references to the developer's file system. To support this, Thumb implements a virtual filesystem mechanism.

The virtual filesystem stores assets with relative path names, overlaying a list of data sources that may include user directories, ZIP archives, and statically-linked binaries. This overlay allows user-defined assets to override application assets freely. Of course, normal access to the root file system remains available along side the virtual file system.

File selection dialogs display the current directory in a text edit box at the top. The root file system is accessed for all absolute paths (directories begining with `/` or `C:/`), and the virtual file system is used for all relative paths. Thus, to view the root filesystem, simply change the directory to `/` and press enter. To view the virtual file system, clear the directory text edit box.
