set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/models")
set(DEST_DIR "${CMAKE_BINARY_DIR}/bin/models")

add_custom_target(copy_data
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${SOURCE_DIR}" "${DEST_DIR}"
    COMMENT "Copying demo assets to ${DEST_DIR}"
)

set_target_properties(copy_data PROPERTIES FOLDER "Tools")
