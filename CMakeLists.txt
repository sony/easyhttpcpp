cmake_minimum_required(VERSION 3.0)

project(easyhttpcpp)

option(EASYHTTPCPP_VERBOSE_MESSAGES "Enable informational messages during configure" OFF)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Generate debug library name with a postfix.")

# set default build type to Release
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Project's CMAKE_BUILD_TYPE" FORCE)
endif ()

include(DefineProjectVersion)
include(DefineProjectMacros)

#############################################################
# various library configurations

option(ENABLE_TESTS
       "Set to OFF|ON (default is ON) to control building tests (requires GTest)" OFF)

option(ENABLE_DOCS
       "Set to OFF|ON (default is ON) to control building api docs (requires Doxygen)" OFF)

option(FORCE_SHAREDLIB
       "Set to OFF|ON (default is ON) to force build project as SHARED library" OFF)

if (FORCE_SHAREDLIB)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(LIB_MODE SHARED)
    if (EASYHTTPCPP_VERBOSE_MESSAGES)
        message(STATUS "Building shared ${CMAKE_PROJECT_NAME} libraries")
    endif ()
else ()
    set(LIB_MODE STATIC)
    if (EASYHTTPCPP_VERBOSE_MESSAGES)
        message(STATUS "Building static ${CMAKE_PROJECT_NAME} libraries")
    endif ()
endif ()

#############################################################
# build & install library target

set(LIBRARY_TARGET_NAME "easyhttp")

# dependent libraries
find_package(OpenSSL REQUIRED)
find_package(Poco REQUIRED Foundation Data DataSQLite Net NetSSL Crypto CONFIG)

# sources
file(GLOB_RECURSE SRCS_G "src/*.cpp")
list(APPEND SRCS ${SRCS_G})
file(GLOB_RECURSE HDRS_G "include/*.h")
list(APPEND SRCS ${HDRS_G})

add_library(${LIBRARY_TARGET_NAME} ${LIB_MODE} ${SRCS})
set_target_properties(${LIBRARY_TARGET_NAME}
                      PROPERTIES
                      VERSION ${PROJECT_VERSION} SOVERSION ${SHARED_LIBRARY_VERSION}
                      OUTPUT_NAME ${LIBRARY_TARGET_NAME}
                      )

target_link_libraries(${LIBRARY_TARGET_NAME}
                      Poco::Foundation
                      Poco::Data Poco::DataSQLite
                      Poco::Net Poco::NetSSL Poco::Crypto
                      OpenSSL::SSL OpenSSL::Crypto)

target_include_directories(${LIBRARY_TARGET_NAME}
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:include>
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
                           )

easyhttpcpp_install(${LIBRARY_TARGET_NAME})
easyhttpcpp_generate_package(${LIBRARY_TARGET_NAME})

#############################################################
# build & install test target

if (ENABLE_TESTS)
    include(CTest)
    enable_testing()

    add_subdirectory(tests)
    if (EASYHTTPCPP_VERBOSE_MESSAGES)
        message(STATUS "Building with tests.")
    endif ()
else ()
    if (EASYHTTPCPP_VERBOSE_MESSAGES)
        message(STATUS "Building without tests.")
    endif ()
endif ()

#############################################################
# build & install docs

if (ENABLE_DOCS)
    if (EASYHTTPCPP_VERBOSE_MESSAGES)
        message(STATUS "Building with docs.")
    endif ()
    find_package(Doxygen REQUIRED)
    if (DOXYGEN_FOUND)
        set(DOC_INPUT_DIRS "${PROJECT_SOURCE_DIR}/include")

        add_subdirectory(doc)
    endif ()
else ()
    if (EASYHTTPCPP_VERBOSE_MESSAGES)
        message(STATUS "Building without docs.")
    endif ()
endif ()

#############################################################
# cmake config files

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

configure_file(cmake/DefineProjectConfig.cmake.in
               "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}Config.cmake" @ONLY)
install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
        DESTINATION
        "lib/cmake/${CMAKE_PROJECT_NAME}"
        COMPONENT
        Devel
)

#############################################################
# verbose messages

if (EASYHTTPCPP_VERBOSE_MESSAGES)
    message(STATUS "CMake ${CMAKE_VERSION} successfully configured ${PROJECT_NAME} using ${CMAKE_GENERATOR} generator")
    message(STATUS "Installation target path: ${CMAKE_INSTALL_PREFIX}")
    message(STATUS "LD_LIBRARY_PATH:=${LD_LIBRARY_PATH}")

    message(STATUS "CMAKE_C_FLAGS:=${CMAKE_C_FLAGS}")
    message(STATUS "CMAKE_C_FLAGS_DEBUG:=${CMAKE_C_FLAGS_DEBUG}")
    message(STATUS "CMAKE_C_FLAGS_RELEASE:=${CMAKE_C_FLAGS_RELEASE}")
    message(STATUS "CMAKE_C_FLAGS_MINSIZEREL:=${CMAKE_C_FLAGS_MINSIZEREL}")
    message(STATUS "CMAKE_C_FLAGS_RELWITHDEBINFO:=${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    message(STATUS "")
    message(STATUS "")
    message(STATUS "CMAKE_CXX_FLAGS:=${CMAKE_CXX_FLAGS}")
    message(STATUS "CMAKE_CXX_FLAGS_DEBUG:=${CMAKE_CXX_FLAGS_DEBUG}")
    message(STATUS "CMAKE_CXX_FLAGS_RELEASE:=${CMAKE_CXX_FLAGS_RELEASE}")
    message(STATUS "CMAKE_CXX_FLAGS_MINSIZEREL:=${CMAKE_CXX_FLAGS_MINSIZEREL}")
    message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO:=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif ()
