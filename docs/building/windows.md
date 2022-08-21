# Building on Windows

# Install Required Utils

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

## Building Amber

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
