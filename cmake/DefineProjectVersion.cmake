# Defines the project version
# Defines for both static and shared libraries
#
# Will define the following variables:
#  PACKAGE_VERSION
#  PROJECT_VERSION
#  PROJECT_VERSION_MAJOR
#  PROJECT_VERSION_MINOR
#  PROJECT_VERSION_PATCH
#  PROJECT_VERSION_EXT
#  SHARED_LIBRARY_VERSION

if (EASYHTTPCPP_VERBOSE_MESSAGES)
    message(STATUS)
    message(STATUS "################# ${PROJECT_NAME} version details #################")
endif ()

file(STRINGS "${PROJECT_SOURCE_DIR}/libversion" SHARED_LIBRARY_VERSION)

# Read the version information from the VERSION file
file(STRINGS "${PROJECT_SOURCE_DIR}/VERSION" PACKAGE_VERSION)
add_definitions(-DPACKAGE_VERSION=${PACKAGE_VERSION})

if (EASYHTTPCPP_VERBOSE_MESSAGES)
    message(STATUS "${PROJECT_NAME} package version: ${PACKAGE_VERSION}")
endif ()

string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" CPACK_PACKAGE_VERSION_MAJOR ${PACKAGE_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1" CPACK_PACKAGE_VERSION_MINOR ${PACKAGE_VERSION})
string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" CPACK_PACKAGE_VERSION_PATCH ${PACKAGE_VERSION})
if (PACKAGE_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    # version has no extensions
    set(CPACK_PACKAGE_VERSION_EXT "")
else ()
    string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.[0-9]+-(.*)" "\\1" CPACK_PACKAGE_VERSION_EXT ${PACKAGE_VERSION})
endif ()

add_definitions(-DPACKAGE_VERSION_MAJOR=${CPACK_PACKAGE_VERSION_MAJOR})
add_definitions(-DPACKAGE_VERSION_MINOR=${CPACK_PACKAGE_VERSION_MINOR})
add_definitions(-DPACKAGE_VERSION_PATCH=${CPACK_PACKAGE_VERSION_PATCH})
add_definitions(-DPACKAGE_VERSION_EXT=${CPACK_PACKAGE_VERSION_EXT})

if (EASYHTTPCPP_VERBOSE_MESSAGES)
    message(STATUS "${PROJECT_NAME} package version major: ${CPACK_PACKAGE_VERSION_MAJOR}")
    message(STATUS "${PROJECT_NAME} package version minor: ${CPACK_PACKAGE_VERSION_MINOR}")
    message(STATUS "${PROJECT_NAME} package version patch: ${CPACK_PACKAGE_VERSION_PATCH}")
    message(STATUS "${PROJECT_NAME} package version extensions: ${CPACK_PACKAGE_VERSION_EXT}")
endif ()

set(PROJECT_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})

if (EASYHTTPCPP_VERBOSE_MESSAGES)
    message(STATUS "${PROJECT_NAME} project version: ${PROJECT_VERSION}")
    message(STATUS "################# ${PROJECT_NAME} version details #################")
    message(STATUS)
endif ()
