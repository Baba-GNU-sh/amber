Glot in an example project showing how lines of varying thicknesses can be drawn using geometry shaders.

## Debian/Ubuntu
Install deps:
```
sudo apt install git cmake build-essential libgl-dev libxinerama-dev libxcursor-dev libxi-dev libglm-dev
```

```
git clone https://github.com/stevegolton/glot.git
mkdir glot/build && cd glot/build
cmake .. && make -j`nproc`
```

Run it with:
```
./glot
```

You should see something like this:
![screenshot](screenshot.png)

## Windows

First make sure to install git and cmake & make sure to add them to your path.
Also install Visual Studio Community edition - has been tested with the 2019 version.

```
git clone https://github.com/stevegolton/glot.git
mkdir glot/build
cd glot/build
cmake ..
```

This should generate the visual studio build dirs in glot/build. You should be able to open the solution with VS Community and build it using the normal methods.


