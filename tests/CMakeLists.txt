#############################################################
# build & install testutil utility library

set(TESTUTIL_TARGET_NAME ${LIBRARY_TARGET_NAME}-testutil)

# dependent libraries
find_package(GMock CONFIG REQUIRED)

# sources
file(GLOB SRCS_G "testutil/*.cpp")

add_library(${TESTUTIL_TARGET_NAME} ${LIB_MODE} ${SRCS_G})

target_link_libraries(${TESTUTIL_TARGET_NAME}
                      easyhttp
                      GMock::main
                      Poco::Foundation
                      Poco::Data Poco::DataSQLite
                      Poco::Net Poco::NetSSL Poco::Crypto
                      OpenSSL::SSL OpenSSL::Crypto)

target_include_directories(${TESTUTIL_TARGET_NAME}
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/testutil
                           )

#############################################################
# build & install test targets

include(CMakeLists-unittests.txt)
include(CMakeLists-integrationtests.txt)
