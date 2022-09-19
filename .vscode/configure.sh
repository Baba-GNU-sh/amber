#!/bin/bash

die() {
  echo "ERROR: $1"
  exit 1
}

BUILD_TYPE="$1"
BUILD_DIR="build/${BUILD_TYPE}"

MODE="Release"
COVERAGE="OFF"
SANITIZERS="OFF"
TESTS="OFF"
BENCHMARKS="OFF"
IWYU="OFF"

case "$BUILD_TYPE" in
  "Debug")
    MODE="Debug"
    ;;
  "Release")
    MODE="Release"
    ;;
  "Test")
    MODE="Debug"
    TESTS="ON"
    ;;
  "Test")
    MODE="Debug"
    TESTS="ON"
    ;;
  "TestCoverage")
    MODE="Debug"
    TESTS="ON"
    COVERAGE="ON"
    ;;
  "Benchmark")
    MODE="Release"
    BENCHMARK="ON"
    ;;
  "IWYU")
    MODE="Release"
    IWYU="ON"
    ;;
  *)
    die "Select one of: Debug, Release, Test, TestCoverage, Benchmark, IWYU"
    ;;
esac

echo "Build settings: ${BUILD_TYPE} Coverage=${COVERAGE} Sanitizers=${SANITIZERS} Tests=${TESTS} Benchmarks=${BENCHMARKS} IWYU=${IWYU}"

mkdir -p "${BUILD_DIR}"
ln -sfn "${BUILD_TYPE}" "build/current"
CONAN_SYSREQUIRES_MODE=enabled conan install -if "${BUILD_DIR}" -s build_type="${MODE}" .
cmake \
  -B "${BUILD_DIR}" \
  -DCMAKE_MODULE_PATH="${PWD}/${BUILD_DIR}" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
  -DCMAKE_BUILD_TYPE="${MODE}" \
  -DBUILD_TESTS="${TESTS}" \
  -DBUILD_BENCHMARKS="${BENCHMARKS}" \
  -DUSE_COVERAGE="${COVERAGE}" \
  -DUSE_SANITIZERS="${SANITIZERS}" \
  -DENABLE_IWYU="${IWYU}" .
ln -sfn build/current/compile_commands.json compile_commands.json
