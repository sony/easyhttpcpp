#!/bin/bash

set -e

CWD=`pwd`

POCO_VERSION=poco-1.7.9-release
GOOGLETEST_VERSION=1.8.0-hunter-p2

rm -rf _install

rm -rf _external && mkdir _external
cd _external

# download poco source
wget https://github.com/pocoproject/poco/archive/${POCO_VERSION}.zip

# extract archive
unzip ${POCO_VERSION}.zip >/dev/null
cd poco-${POCO_VERSION}

# install Poco as a static library in release mode
CMAKE_TOOLCHAIN_FILE_ARG=""
if [[ "`uname`" == "Darwin" ]]; then
    # workaround to find_package(OpenSSL)
    CMAKE_TOOLCHAIN_FILE_ARG="-DCMAKE_TOOLCHAIN_FILE=${CWD}/cmake/AppleToolchain.cmake"
fi

mkdir _build && cd _build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=11 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DPOCO_VERBOSE_MESSAGES=OFF -DPOCO_STATIC=ON -DPOCO_UNBUNDLED=ON -DENABLE_XML=ON -DENABLE_JSON=ON -DENABLE_UTIL=ON -DENABLE_NET=ON -DENABLE_NETSSL=ON -DENABLE_CRYPTO=ON -DENABLE_DATA=ON -DENABLE_DATA_SQLITE=ON -DENABLE_ZIP=OFF -DENABLE_DATA_MYSQL=OFF -DENABLE_DATA_ODBC=OFF -DENABLE_MONGODB=OFF -DENABLE_PDF=OFF -DENABLE_SEVENZIP=OFF -DENABLE_PAGECOMPILER=OFF -DENABLE_PAGECOMPILER_FILE2PAGE=OFF -DENABLE_TESTS=OFF -DCMAKE_INSTALL_PREFIX=${CWD}/_install ${CMAKE_TOOLCHAIN_FILE_ARG} ../
make -j4 install >/dev/null

cd ${CWD}
cd _external

# download google test source (use Hunter packages since theirs support cmake config generation)
wget https://github.com/hunter-packages/googletest/archive/${GOOGLETEST_VERSION}.zip

# extract google test source
unzip ${GOOGLETEST_VERSION}.zip >/dev/null
cd googletest-${GOOGLETEST_VERSION}

# install Google Test as a static library in release mode
mkdir _build && cd _build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${CWD}/_install ../
make -j4 install >/dev/null

cd ${CWD}
