[![pipeline status](https://gitlab.com/tinker3/glot/badges/master/pipeline.svg)](https://gitlab.com/tinker3/glot/-/commits/master)
[![coverage report](https://gitlab.com/tinker3/glot/badges/master/coverage.svg)](https://gitlab.com/tinker3/glot/-/commits/master)


Below is a sample image from the latest branch of the build artifacts.
![sample image from the](https://gitlab.com/tinker3/glot/-/jobs/artifacts/master/raw/public/html/bdwn.png?job=pages)

Documentation can be found under [pages](https://tinker3.gitlab.io/glot/).

Glot is an example project showing how high frequency time series data can be plotted using a line of arbitrary thickness with error bars, rendered on the GPU.

Glot uses [conan](http://conan.io/) to manage its dependencies, which makes it easier / possible to build it on Windows (and hopefully MacOS).

# Building
Glot can be built on Debian/Ubuntu linux as well as Windows 10.

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

