cmake_minimum_required(VERSION 3.15)
project(Solution LANGUAGES C CXX)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()


add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)
add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)
add_compile_definitions($<$<CONFIG:Release>:NDEBUG1>)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)

if (MSVC)
    add_compile_options(/EHsc)

    add_compile_options(
        "$<$<CONFIG:Debug>:/Zi;/Od>"
        "$<$<CONFIG:Release>:/Zi;/O2>"
    )

    add_link_options("$<$<CONFIG:Release>:/DEBUG>")

else()
    add_compile_options(
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-g;-O0>
        $<$<CONFIG:Release>:-O3>
    )
endif()

add_compile_definitions(
    $<$<CONFIG:Debug>:DEBUG>
)
add_subdirectory(commons)
add_subdirectory(interface_menu)
add_subdirectory(text)
add_subdirectory(physic_engine)
add_subdirectory(vulkan_struct)
add_subdirectory(asset_loader)
add_subdirectory(vulkan_engine)
add_subdirectory(window_app)
add_subdirectory(shaders)
add_subdirectory(qt_gui_render)
add_subdirectory(sdl_app)



