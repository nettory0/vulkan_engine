#cmake_minimum_required(VERSION 3.15)
#project(CrossPlatformExample LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

include(FetchContent)

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
    poly2tri
    GIT_REPOSITORY https://github.com/jhasse/poly2tri.git
    GIT_TAG master
)
FetchContent_MakeAvailable(poly2tri)

if(TARGET poly2tri)
    set_target_properties(poly2tri PROPERTIES FOLDER "External")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG master
)
FetchContent_MakeAvailable(freetype)

if(TARGET freetype)
    set_target_properties(freetype PROPERTIES FOLDER "External")
endif()

add_library(text_gen SHARED
    tg_mesh.h
    tg_text_assistent.cpp
	tg_text_assistent.h
    tg_text_export.h
    tg_font2OBJ.h
    tg_shapes.cpp
    tg_sweep.cpp
    tg_shapes.h
    tg_sweep.h
    tg_sweep_context.cpp
    tg_sweep_context.h
    tg_utils.h
    tg_vectoriser.cpp
    tg_vectoriser.h
)

set_target_properties(text_gen PROPERTIES FOLDER "Project")

set_target_properties(text_gen PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
)

target_compile_definitions(text_gen PRIVATE TEXT_GEN_EXPORT)
target_include_directories(text_gen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


target_include_directories(text_gen
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  
       
)


target_link_libraries(text_gen 
    PRIVATE glm::glm-header-only
    PRIVATE poly2tri
    PRIVATE freetype
)
