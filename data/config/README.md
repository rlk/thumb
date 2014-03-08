# Thumb Host Configurations

Host configuration files define the window size and position, the viewport, the display type and dimensions, the off-screen render size, the heads-up-display configuration, the cluster configuration, etc. They are organized by use.

-   `common`

	These are ordinary desktop and laptop configurations. They come in a variety of sizes, and provided windowed and non-windowed versions. Start here.

-   `stereoscopic`

	These configurations support stereoscopic TVs and basic virtual reality devices.

-   `production`

	Production configurations are used to render finished image sequences. They usually provide a low-resolution preview backed by a high-resolution off-screen buffer.

-   `cluster`

	Cluster configurations define distributed renderers. These are usually very unique and highly specialized. Each is designed for a very specific installation.

-   `development`

	These enable the testing and development of host and display support code.

Many many display configurations have come and gone through Thumb's history. Don't expect everything in this directory to be useful. In particular, if you try running a cluster configuration on a system other than the one it was designed for, you'll probably just hang on a waiting socket.
