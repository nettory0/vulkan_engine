set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(DEST_DIR "${CMAKE_BINARY_DIR}/bin/")

if (NOT EXISTS "${SOURCE_DIR}")
    message(FATAL_ERROR "Source directory does not exist: ${SOURCE_DIR}")
endif()

message(STATUS "SOURCE_DIR = ${SOURCE_DIR}")
message(STATUS "DEST_DIR   = ${DEST_DIR}")
message(STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")


add_custom_target(copy_data
    COMMAND ${CMAKE_COMMAND} -E echo "Copying FROM: ${SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Copying TO:   ${DEST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${SOURCE_DIR}" "${DEST_DIR}"
    COMMENT "Copying data"
)

set_target_properties(copy_data PROPERTIES FOLDER "Tools")