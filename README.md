Glot in an example project showing how graphs can be plotted using arbitrary thickness lines along error (noise) bars can be drawn efficiently using geometry shaders.

## Debian/Ubuntu
Install deps:
```
sudo apt install git cmake build-essential libgl-dev libxinerama-dev libxcursor-dev libxi-dev
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

Use the scroll wheel to zoom in and out, and use the left mouse button to drag the canvas around.

## Windows

First make sure to install git and cmake & make sure to add them to your path.
Also install Visual Studio Community edition - has been tested with the 2019 version.

```
git clone https://github.com/stevegolton/glot.git
mkdir glot\build
cd glot\build
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


