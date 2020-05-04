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
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}${target_name}ConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
    )
    export(EXPORT "${target_name}Targets"
           FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}${target_name}Targets.cmake"
           NAMESPACE "${PROJECT_NAME}::"
           )

    configure_file("${PROJECT_MODULE_PATH}/${PROJECT_NAME}${target_name}Config.cmake.in"
                   "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}${target_name}Config.cmake"
                   @ONLY
                   )

    set(ConfigPackageLocation "lib/cmake/${PROJECT_NAME}")

    install(
            EXPORT "${target_name}Targets"
            FILE "${PROJECT_NAME}${target_name}Targets.cmake"
            NAMESPACE "${PROJECT_NAME}::"
            DESTINATION "lib/cmake/${PROJECT_NAME}"
    )

    install(
            FILES
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}${target_name}Config.cmake"
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}${target_name}ConfigVersion.cmake"
            DESTINATION "lib/cmake/${PROJECT_NAME}"
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
