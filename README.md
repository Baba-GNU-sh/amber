```
   ___         __          
  / _ | __ _  / /  ___ ____
 / __ |/  ' \/ _ \/ -_) __/
/_/ |_/_/_/_/_.__/\__/_/
 Real Time Data Plotter
```

[![Actions Status](https://github.com/baba-GNU-sh/glot/workflows/ci/badge.svg?branch=main)](https://github.com/baba-GNU-sh/glot/actions)
[![codecov](https://codecov.io/gh/baba-GNU-sh/glot/branch/main/graph/badge.svg?token=TYEKEONCCL)](https://codecov.io/gh/baba-GNU-sh/glot)

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

## Ubuntu

Install python, then install conan:

```bash
sudo apt install python3-pip build-essential cmake git
pip3 install conan
conan profile update settings.compiler.libcxx=libstdc++11 default
```

Clone this repo then cd into the root:

```bash
mkdir build && cd build
CONAN_SYSREQUIRES_MODE=enabled conan install ..
cmake -DCMAKE_MODULE_PATH=${PWD} -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

Run Amber with:

```bash
./amber
```

## MacOS

First, install the developer tools for mac, which should install things like clang and make. Then install homebrew, and with it install conan and cmake.

```zsh
brew install conan cmake
```

Clone this repo, and from within the root of the cloned workspace, run:

```zsh
mkdir build && cd build
conan install .. -s build_type=Debug --build missing
cmake -DCMAKE_MODULE_PATH=${PWD} -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

Run Amber with:

```zsh
./amber
```

## Windows

### Install Required Tools

Download and install the following tools:

- `cmake` <https://cmake.org/download/>
- `conan` <https://conan.io/downloads.html>

Make sure `cmake`, and `conan` are available in your `PATH`. Specifically you should be able to run `cmake --version` and `conan --version` in cmd or PowerShell and get something that looks like this:

```powershell
PS C:\Users\steve> cmake --version
cmake version 3.22.1

CMake suite maintained and supported by Kitware (kitware.com/cmake).
PS C:\Users\steve> conan --version
Conan version 1.44.1
```

You will also need to get hold of the MSVC compiler from somewhere. I would recommend installing Visual Studio Community Edition. Amber has been tested with the 2019 or 2022 editions.

Clone this repo then cd into into the newly created dir. Now rung the following to make a build:

```powershell
mkdir build
cd build
conan install .. -s build_type=Debug
cmake "-DCMAKE_MODULE_PATH=$((pwd).path -replace '\\', '/')" ..
cmake --build . --config Debug
```

Now you can run Amber with the following command:

```powershell
.\build\Debug\amber.exe
```

Cmake should have generated a VS solution in `build/amber.sln` which you can open up in Visual Studio, and debug and build directly from there. Just make sure to select the `main` project as the startup application.

## Running Tests

Amber comes with a suite of tests and performance benchmarks.

> Note: this only works with `gcc` on Linux AFAIK.

```bash
mkdir build && cd build
CONAN_SYSREQUIRES_MODE=enabled conan install ..
cmake -DCMAKE_MODULE_PATH=${PWD} -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DUSE_COVERAGE=ON -DUSE_SANITIZERS=ON ..
cmake --build .
ctest --verbose
```

Generate a report using gcovr, and opening the results in a web browser

```bash
gcovr --root .. --html-details -o coverage.html
firefox coverage.html
```

## Running Benchmarks

Performance is an important part of amber, so some benchmarks are also available, mainly oriented around accessing timeseries data.

```bash
mkdir build && cd build
CONAN_SYSREQUIRES_MODE=enabled conan install ..
cmake -DCMAKE_MODULE_PATH=${PWD} -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON ..
cmake --build .
ctest --verbose
```

## Alternatives

- [PlotJuggler](https://www.plotjuggler.io/)

## Additional Notes

### Using with clangd

```bash
cd build
cmake -DCMAKE_MODULE_PATH=${PWD} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
```

From the root dir, make a symlink to `compile_commands.json` in the build dir.
```
ln -s build/compile_commands.json .
```

## Fonts
Amber uses the font ProggyClean by Tristan Grimmer. This is pre-rendered out to a bitmap font atlas in `font.png`.

Ascii art from https://patorjk.com/software/taag/#p=display&f=Small%20Slant&t=Amber
