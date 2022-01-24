![example workflow](https://github.com/Baba-GNU-sh/glot/actions/workflows/ci.yml/badge.svg)

[![coverage report](https://gitlab.com/tinker3/glot/badges/master/coverage.svg)](https://gitlab.com/tinker3/glot/-/commits/master)
[![pipeline status](https://gitlab.com/tinker3/glot/badges/master/pipeline.svg)](https://gitlab.com/tinker3/glot/-/commits/master)

Glot is an example project showing how high frequency time series data can be plotted using a line of arbitrary thickness and error bars, all drawn on the GPU using using geometry shaders.

Glot uses conan to manage its dependencies. The reason I am using using conan rather than the package manager is to make it easier / possible to build on Windows and Mac.

## Debian/Ubuntu
Install python, then install conan:
```bash
sudo apt install python3-pip build-essential cmake git
pip3 install conan
```

Configure conan to use the C++11 abi compatible version of the standard library.
```bash
conan profile update settings.compiler.libcxx=libstdc++11 default
```

Clone this repo then cd into the root:
```bash
mkdir build && cd build
CONAN_SYSREQUIRES_MODE=enabled conan install ..
cmake -DCMAKE_MODULE_PATH=${PWD} ..
cmake --build .
ctest
```

Run it with:
```
bin/glot
```

You should see something like this:
![screenshot](screenshot.png)

Use the scroll wheel to zoom in and out, and use the left mouse button to drag the canvas around.

## Windows

First make sure to install `git` and `cmake` and make sure they are in your `PATH`.

Make sure to install Visual Studio (Community Edition will do) - glot has been tested with the 2019 version. Cmake should pick this up automatically.

Install conan using the installer https://conan.io/downloads.html or using pip if you have python installed already.

Clone this repo then cd into the root:
> Note: GTest doesn't appear to work on Windows, so we just disable testing with `-DBUILD_TESTS=OFF`
```ps
mkdir build
cd build
conan install ..
cmake "-DCMAKE_MODULE_PATH=$((pwd).path -replace '\\', '/')" -DBUILD_TESTS=OFF ..
cmake --build .
```

If compilation on the command line isn't your bag, cmake should have generated a VS project in `glot/build/glot.vcxproj` which you can open up in Visual Studio and tinker with.

You can run the executable using:
```ps
.\Debug\glot.exe
```

Or of you built it in release mode:
```ps
.\Release\glot.exe
```

As in the linux version, use the scroll wheel to zoom in and out, and use the left mouse button to drag the canvas around.

# Using with clangd
```
cd build
cmake -DCMAKE_MODULE_PATH=${PWD} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
```

From the root dir, make a symlink to `compile_commands.json` in the build dir.
```
ln -s build/compile_commands.json .
```

