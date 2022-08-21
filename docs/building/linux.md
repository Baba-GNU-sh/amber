# Building on Linux

## Ubuntu/Debian

## Install Required Utils

Install python, then install conan:

```bash
sudo apt install python3-pip build-essential cmake git
pip3 install conan
conan profile update settings.compiler.libcxx=libstdc++11 default
```

### Building Amber

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

### Running Tests

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

### Running Benchmarks

Performance is an important part of amber, so some benchmarks are also available, mainly oriented around accessing timeseries data.

```bash
mkdir build && cd build
CONAN_SYSREQUIRES_MODE=enabled conan install ..
cmake -DCMAKE_MODULE_PATH=${PWD} -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON ..
cmake --build .
ctest --verbose
```
