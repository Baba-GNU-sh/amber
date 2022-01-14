![example workflow](https://github.com/Baba-GNU-sh/glot/actions/workflows/ci.yml/badge.svg)

Glot is an example project showing how high frequency time series data can be plotted using a line of arbitrary thickness and error bars, all drawn on the GPU using using geometry shaders.

Glot uses conan to manage its dependencies. The reason I am using using conan rather than the package manager is to make it easier / possible to build on Windows and Mac.

## Debian/Ubuntu
Install python, then install conan:
```
sudo apt install python3-pip build-essential cmake git
pip3 install conan
```

Configure conan to use the C++11 abi compatible version of the standard library.
```
conan profile update settings.compiler.libcxx=libstdc++11 default
```

Clone this repo then cd into the root:
```
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

> Note: This needs testing!

First make sure to install git and cmake & make sure to add them to your path.
Also install Visual Studio Community edition - has been tested with the 2019 version.
Install python then install conan. Make sure you can run `conan` from your terminal.

Clone this repo then cd into the root:
```
mkdir build
cd build
conan install ..
cmake -G"Visual Studio 16 2019" ..
```

This should generate a VS project in `glot/build`. You should be able to open the `glot.vcxproj` with VS Community and build it with F6.
However it seems for some reason VS can't find the executable to run, so run it from the command line like so:
```
.\Debug\glot.exe
```

Or of you built it in release mode:

```
.\Release\glot.exe
```

As in the linux version, the scroll wheel to zoom in and out, and use the left mouse button to drag the canvas around.
