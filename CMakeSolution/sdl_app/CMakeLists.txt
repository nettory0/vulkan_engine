cmake_minimum_required(VERSION 3.15)
project(CrossPlatformExample LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ---------------- VOLK ----------------

FetchContent_Declare(
    volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
    GIT_TAG master
)

FetchContent_MakeAvailable(volk)

# ---------------- SDL3 ----------------

set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.14
)

FetchContent_MakeAvailable(SDL3)

foreach(target
    SDL3
    SDL3-static
    SDL_uclibc
    SDL3_test
    SDL3-shared
)
    if(TARGET ${target})
        set_target_properties(${target} PROPERTIES
            FOLDER "External/SDL3"
        )
    endif()
endforeach()

# ---------------- Vulkan ----------------

find_package(Vulkan REQUIRED)

# ---------------- APP ----------------

add_executable(sdl_app
    main.cpp
    app.h
    app.cpp
    sdl_window.h
    sdl_window.cpp
    surface.cpp
    surface.h
)


set_target_properties(sdl_app PROPERTIES FOLDER "Project")

set_target_properties(sdl_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
)

# debugger working dir


target_compile_definitions(sdl_app PRIVATE VK_NO_PROTOTYPES)
target_include_directories(sdl_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${volk_SOURCE_DIR}       
    ${glm_SOURCE_DIR}       
)

target_link_libraries(sdl_app
    PRIVATE vulkan_engine
        PRIVATE
        SDL3::SDL3
        Vulkan::Vulkan
        volk
    PRIVATE assimp
    PRIVATE freetype
    PRIVATE Vulkan::Vulkan
)

set_target_properties(sdl_app PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:sdl_app>"
)