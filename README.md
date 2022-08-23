```
   ___         __          
  / _ | __ _  / /  ___ ____
 / __ |/  ' \/ _ \/ -_) __/
/_/ |_/_/_/_/_.__/\__/_/
 Real Time Data Plotter
```

[![Actions Status](https://github.com/baba-GNU-sh/amber/workflows/ci/badge.svg?branch=main)](https://github.com/baba-GNU-sh/amber/actions)
[![codecov](https://codecov.io/gh/baba-GNU-sh/amber/branch/main/graph/badge.svg?token=TYEKEONCCL)](https://codecov.io/gh/baba-GNU-sh/amber)

> :warning: **This project is under heavy development, and thus is unlikely to be very useful to anyone in its current state!**

# Introduction

Amber is (going to be) a real-time data-plotter for Windows, Linux and Mac, designed to be used for exploring high-frequency time-series data.

Examples of data sources Amber coulde be good for are: robots, servers, data loggers, sound cards, USB oscillioscopes, and USB logic analyzers.

Much like an oscilloscope, Amber displays signals on a 2D graph, with time along the X axis and signal value up the Y axis. Amber provides intuitive controls for navigating the graph (e.g. scrolling around, zooming in and out on combined & individual axes, zooming in on specific events with a selection tool), and remains fast and responsive even when displaying large amounts of high frequency data.

The graph renderer is be designed to give you as much information about the signal as possible at every zoom level without aliasing, for example, by showing average value + error bars when there is more than data point per pixel.

Ultimately, data sources shall be provided via plugins, allowing users to easily get data from custom sources into Amber.

# Rationale

Amber is designed to fill a hole that I have personally encountered many times while working as an embedded software engineer: I can get sensor data out of my embedded device but have no decent way to view and explore it! There are alternatives (see the [Alternatives](#alternatives) section for a list), and while there are plenty of quality tools out there, I've found all of them to be lacking in at least one important way.

At the time of writing, Amber is far from complete. It's mainly an example project which I am using to test the graph renderer (which is based on OpenGL), and to figure out the best way to draw other UI elements such as menus and buttons, for which I am currently using ImGui.

# Design Philosophy

Amber is designed around the following principles:

- Keep recording data 'til your memory is full (no fixed size ring buffer here).
- Always show the most useful information possible, regardless of the zoom level without aliasing.
- Target 60FPS even on modest integrated laptop graphics.
- Builds and runs on Windows, Linux, and Mac - see [actions](/actions).

# Screenshots

Ubuntu
![screenshot_ubuntu](screenshot_ubuntu.png)

Mac
![screenshot_maxos](screenshot_macos.png)

Windows
![screenshot_macos](screenshot_windows.png)


# Building from source

Amber uses [conan](http://conan.io/) to manage dependencies, which makes the build easier to manage on different platforms.

Amber has been tested on Ubuntu 20.04, Windows 10, and MacOS Monterey, but it may well work on other distros and OS versions.

Follow the relevant link below to find build instructions for your system.

- [MacOS](docs/building/macos.md)
- [Linux](docs/building/linux.md)
- [Windows](docs/building/windows.md)

# Development

I would highly recommend using clangd to develop Amber, as it integrates nicely with cmake. You can ask cmake to generate a `compile_commands.json` file which is read up by clangd so autocompletion and highlighting can be tightly integrated with the build system.

Make sure `clangd` is available on your path, by installing clangd using your package manager, and install a relevant extension for your IDE, such as the clangd extension for VSCode or ALE for vim.

Then add `-DCMAKE_EXPORT_COMPILE_COMMANDS=1` to your cmake command, and make a symlink from the root of the repo to this file in your build dir `ln -s build/compile_commands.json .`.

# Alternatives

- [PlotJuggler](https://www.plotjuggler.io/)
- todo...


# Acknowledgements

Amber uses the font ProggyClean by Tristan Grimmer. This is pre-rendered out to a bitmap font atlas in and can be found in `assets/fonts/proggy_clean.png`. 

The Amber ascii art at the top of this README is from here: https://patorjk.com/software/taag/#p=display&f=Small%20Slant&t=Amber
