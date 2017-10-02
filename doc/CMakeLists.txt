if (DOXYGEN_FOUND)
    set(DOC_OUTPUT_DIR ${CMAKE_INSTALL_PREFIX}/doc/api CACHE PATH "API documentation directory")
    set(DOXYGEN_OUTPUT ${APIDOC_DIR}/html/index.html)

    set(DOXYFILE_IN ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in)
    set(DOXYFILE ${CMAKE_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYFILE_IN} ${DOXYFILE} @ONLY)

    file(MAKE_DIRECTORY ${DOC_OUTPUT_DIR}/html)

    add_custom_target(doc
                      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Generating API documentation with Doxygen"
                      VERBATIM
                      )
endif (DOXYGEN_FOUND)
