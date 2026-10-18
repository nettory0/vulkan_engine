#cmake_minimum_required(VERSION 3.15)
#project(CrossPlatformExample LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

include(FetchContent)

set(YAML_MSVC_SHARED_RT ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG master
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)
if(TARGET glm)
    set_target_properties(glm PROPERTIES FOLDER "External")
endif()

set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(yaml-cpp)

if(TARGET yaml-cpp)
    set_target_properties(yaml-cpp PROPERTIES FOLDER "External")
endif()


add_library(asset_loader SHARED
	asset_loader.h
	asset_loader.cpp
)

 set_target_properties(asset_loader PROPERTIES FOLDER "Project")

set_target_properties(asset_loader PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
)

target_compile_definitions(asset_loader PRIVATE ASSET_EXPORTS)
target_include_directories(asset_loader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(asset_loader
    PRIVATE yaml-cpp::yaml-cpp
    PRIVATE glm::glm-header-only
)