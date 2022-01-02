Glot is a high performance plotting tool and time series database, capable of plotting and exploting high frequency (e.g. kHz) time series data from various sources.

## Building for Windows:

First make sure to install git, and cmake. Make sure to add them to your path.
Also install Visual Studio Community edition. Glot has been tested with the 2019 version.

```
git clone --recurse-submodules https://github.com/stevegolton/glot.git
mkdir glot/build && cd glot/build
cmake ..
```

This should generate the visual studio build dirs in glot/build. You should be able to open the solution with VS Community and build it using the normal methods.

## Building for Debian-based platforms:
Install OpenGL

```
git clone --recurse-submodules https://github.com/stevegolton/glot.git
mkdir glot/build && cd glot/build
cmake .. && make -j`nproc`
```

Run it with:
```
./glot
```
