#===============================================================================
# Macro for Package generation
#
#  easyhttpcpp_generate_package - Generates *Config.cmake
#    Usage: easyhttpcpp_generate_package(target_name)
#      INPUT:
#           target_name     the name of the target.
#    Example: easyhttpcpp_generate_package(Target)
macro(easyhttpcpp_generate_package target_name)
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
            "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}${target_name}ConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
    )
    export(EXPORT "${target_name}Targets"
           FILE "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}${target_name}Targets.cmake"
           NAMESPACE "${CMAKE_PROJECT_NAME}::"
           )

    configure_file("${CMAKE_MODULE_PATH}/${CMAKE_PROJECT_NAME}${target_name}Config.cmake.in"
                   "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}${target_name}Config.cmake"
                   @ONLY
                   )

    set(ConfigPackageLocation "lib/cmake/${CMAKE_PROJECT_NAME}")

    install(
            EXPORT "${target_name}Targets"
            FILE "${CMAKE_PROJECT_NAME}${target_name}Targets.cmake"
            NAMESPACE "${CMAKE_PROJECT_NAME}::"
            DESTINATION "lib/cmake/${CMAKE_PROJECT_NAME}"
    )

    install(
            FILES
            "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}${target_name}Config.cmake"
            "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}${target_name}ConfigVersion.cmake"
            DESTINATION "lib/cmake/${CMAKE_PROJECT_NAME}"
            COMPONENT Devel
    )
endmacro()

#===============================================================================
# Macros for simplified installation
#
#  easyhttpcpp_install - Install the given target
#    Usage: easyhttpcpp_install(target_name)
#      INPUT:
#           target_name     the name of the target.
#    Example: easyhttpcpp_install(Target)
macro(easyhttpcpp_install target_name)
    install(
            DIRECTORY include/easyhttpcpp
            DESTINATION include
            COMPONENT Devel
            PATTERN ".gitkeep" EXCLUDE
    )

    install(
            TARGETS "${target_name}" EXPORT "${target_name}Targets"
            LIBRARY DESTINATION lib${LIB_SUFFIX}
            ARCHIVE DESTINATION lib${LIB_SUFFIX}
            RUNTIME DESTINATION bin
            INCLUDES DESTINATION include
    )
endmacro()
