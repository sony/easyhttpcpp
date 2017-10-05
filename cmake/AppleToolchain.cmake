# Cross-compilation loader for Apple
#
# This file is passed to cmake on the command line via
# -DCMAKE_TOOLCHAIN_FILE.

# OpenSSL must be downloaded manually: brew install openssl
execute_process(COMMAND brew --prefix openssl OUTPUT_VARIABLE BREW_OPENSSL_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

set(OPENSSL_ROOT_DIR "${BREW_OPENSSL_DIR}")
set(OPENSSL_INCLUDE_DIR "${BREW_OPENSSL_DIR}/include")

set(CMAKE_INCLUDE_PATH ${OPENSSL_INCLUDE_DIR})
set(CMAKE_LIBRARY_PATH "${OPENSSL_ROOT_DIR}/lib")

include_directories(
        "${OPENSSL_INCLUDE_DIR}"
)

link_directories(
        "${OPENSSL_ROOT_DIR}/lib"
)
