#cmake_minimum_required(VERSION 3.15)
#project(CrossPlatformExample LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

include(FetchContent)

set(YAML_MSVC_SHARED_RT ON CACHE BOOL "" FORCE)


FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glm)
if(TARGET glm)
    set_target_properties(glm PROPERTIES FOLDER "External")
endif()

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0 
)
FetchContent_MakeAvailable(spdlog)

if(TARGET spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER "External")
endif()

add_library(commons SHARED
    co_event.h
	co_export.h
    co_types.h
	co_structs.h
	co_structs.cpp
    co_geom_primitives.h
    co_geom_primitives.cpp
    co_logging.h
    co_logging.cpp
)

 set_target_properties(commons PROPERTIES FOLDER "Project")

set_target_properties(commons PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
)
target_compile_definitions(commons
    PUBLIC COMMON_STRUCT_EXPORTS
    PUBLIC GLM_ENABLE_EXPERIMENTAL
)
target_include_directories(commons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(commons
    PUBLIC glm::glm-header-only
    PRIVATE spdlog::spdlog
)