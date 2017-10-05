#!/bin/bash

set -e

CWD=`pwd`

cd samples

rm -rf _build && mkdir _build
cd _build

CMAKE_TOOLCHAIN_FILE_ARG=""
if [[ "`uname`" == "Darwin" ]]; then
    # workaround to find_package(OpenSSL)
    CMAKE_TOOLCHAIN_FILE_ARG="-DCMAKE_TOOLCHAIN_FILE=${CWD}/cmake/AppleToolchain.cmake"
fi

# build all samples with Debug lib
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=11 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_PREFIX_PATH=${CWD}/_install ${CMAKE_TOOLCHAIN_FILE_ARG} ../
make >/dev/null

# run all samples
./bin/easyhttpcpp-samples-SimpleHttpClient https://github.com/sony/easyhttpcpp

# build all samples with Release lib
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=11 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_PREFIX_PATH=${CWD}/_install ${CMAKE_TOOLCHAIN_FILE_ARG} ../
make >/dev/null

# run all samples
./bin/easyhttpcpp-samples-SimpleHttpClient https://github.com/sony/easyhttpcpp

cd ${CWD}
