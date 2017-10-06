#!/bin/bash

set -e

CWD=`pwd`

rm -rf _build && mkdir _build
cd _build

CMAKE_TOOLCHAIN_FILE_ARG=""
if [[ "`uname`" == "Darwin" ]]; then
    # workaround to find_package(OpenSSL)
    CMAKE_TOOLCHAIN_FILE_ARG="-DCMAKE_TOOLCHAIN_FILE=${CWD}/cmake/AppleToolchain.cmake"
fi

# build easyhttpcpp in Debug mode
cmake -DCMAKE_BUILD_TYPE=Debug -DEASYHTTPCPP_VERBOSE_MESSAGES=ON -DCMAKE_CXX_STANDARD=11 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_PREFIX_PATH=${CWD}/_install -DCMAKE_INSTALL_PREFIX=${CWD}/_install -DENABLE_TESTS=ON ${CMAKE_TOOLCHAIN_FILE_ARG} ../
make -j4 install >/dev/null

# run tests
./bin/easyhttp-UnitTestRunner >/dev/null 2>&1
./bin/easyhttp-IntegrationTestRunner >/dev/null 2>&1

# build easyhttpcpp in Release mode
cmake -DCMAKE_BUILD_TYPE=Release -DEASYHTTPCPP_VERBOSE_MESSAGES=ON -DCMAKE_CXX_STANDARD=11 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_PREFIX_PATH=${CWD}/_install -DCMAKE_INSTALL_PREFIX=${CWD}/_install -DENABLE_TESTS=ON ${CMAKE_TOOLCHAIN_FILE_ARG} ../
make -j4 install >/dev/null

# run tests
./bin/easyhttp-UnitTestRunner >/dev/null 2>&1
./bin/easyhttp-IntegrationTestRunner >/dev/null 2>&1

cd ${CWD}
