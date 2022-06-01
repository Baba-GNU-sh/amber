#!/bin/bash

BUILD_TYPE="$1"

COVERAGE=""
SANITIZERS=""
TESTS=""
BENCHMARKS=""

case "$BUILD_TYPE" in
  "Debug")
    COVERAGE="-DBUILD_COVERAGE=OFF"
    SANITIZERS="-DUSE_SANITIZERS=OFF"
    TESTS="-DBUILD_TESTS=ON"
    BENCHMARKS="-DBUILD_BENCHMARKS=OFF"
    IWYU="-DUSE_IWYU=OFF"
    ;;
  "Release")
    COVERAGE="-DBUILD_COVERAGE=OFF"
    SANITIZERS="-DUSE_SANITIZERS=OFF"
    TESTS="-DBUILD_TESTS=OFF"
    BENCHMARKS="-DBUILD_BENCHMARKS=ON"
    IWYU="-DUSE_IWYU=OFF"
    ;;
esac

echo "Build settings: ${BUILD_TYPE} ${COVERAGE} ${SANITIZERS} ${TESTS} ${BENCHMARKS}"

mkdir -p "build/${BUILD_TYPE}"
ln -sfn "${BUILD_TYPE}" "build/current"
CONAN_SYSREQUIRES_MODE=enabled conan install -if "build/${BUILD_TYPE}" -s build_type="${BUILD_TYPE}" .
cmake -B build/${BUILD_TYPE} -DCMAKE_MODULE_PATH=${PWD}/build/${BUILD_TYPE} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" "${TESTS}" "${COVERAGE}" "${SANITIZERS}" "${BENCHMARKS}" .
ln -sfn build/current/compile_commands.json compile_commands.json
