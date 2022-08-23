# Building on MacOS

First, install the developer tools for mac, which should install things like clang and make. Then install [homebrew](https://brew.sh/), and with it install conan and cmake.

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