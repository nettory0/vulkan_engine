#cmake_minimum_required(VERSION 3.15)
#project(CrossPlatformExample LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  

include(FetchContent)

FetchContent_Declare(
    volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
    GIT_TAG master 
)
FetchContent_MakeAvailable(volk)

if(TARGET volk)
    set_target_properties(volk PROPERTIES FOLDER "External")
endif()

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glfw)

if(TARGET glfw)
    set_target_properties(glfw PROPERTIES FOLDER "External/glfw")
endif()

if(TARGET update_mappings)
    set_target_properties(update_mappings PROPERTIES FOLDER "External/glfw")
endif()


find_package(Vulkan REQUIRED)

add_executable(window_app 
    app.h
    app.cpp
    glfw_window.h
    glfw_window.cpp
    vulkan_main.cpp
    surface.cpp
    surface.h
)

set_target_properties(window_app PROPERTIES FOLDER "Project")

set_target_properties(window_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
)

target_compile_definitions(window_app PRIVATE VK_NO_PROTOTYPES)
target_include_directories(window_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${volk_SOURCE_DIR}       
    ${glm_SOURCE_DIR}       
)

target_link_libraries(window_app
    PRIVATE vulkan_engine
    PUBLIC glfw
    PRIVATE assimp
    PRIVATE freetype
    PRIVATE Vulkan::Vulkan
)

set_target_properties(window_app PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:window_app>"
)
