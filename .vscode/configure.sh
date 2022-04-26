#!/bin/bash

BUILD_TYPE="$1"

mkdir -p "build/${BUILD_TYPE}"
ln -sfn "${BUILD_TYPE}" "build/current"
CONAN_SYSREQUIRES_MODE=enabled conan install -if "build/${BUILD_TYPE}" -s build_type="${BUILD_TYPE}" .
cmake -B build/${BUILD_TYPE} -DCMAKE_MODULE_PATH=${PWD}/build/${BUILD_TYPE} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DBUILD_TESTS=ON .
ln -sfn build/current/compile_commands.json compile_commands.json
