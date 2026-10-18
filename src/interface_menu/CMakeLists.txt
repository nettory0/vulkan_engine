cmake_minimum_required(VERSION 3.15)
project(CrossPlatformExample LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)


FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)

FetchContent_MakeAvailable(stb)


FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG master
)
FetchContent_MakeAvailable(freetype)

if(TARGET freetype AND NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

if(TARGET freetype)
    set_target_properties(freetype PROPERTIES FOLDER "External")
endif()

FetchContent_Declare(
    RmlUi
    GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
    GIT_TAG 6.2
)

FetchContent_MakeAvailable(RmlUi)

foreach(target
    rmlui_core
    rmlui_debugger
)
    if(TARGET ${target})
        set_target_properties(${target} PROPERTIES
            FOLDER "External/RmlUi"
        )
    endif()
endforeach()


# ---------------- APP ----------------

add_library(interface_menu SHARED 
    interface_menu.cpp
    interface_menu.h
    interface_system.h
    interface_system.cpp
    interface_menu_export.h
    interface_types.h
)


set_target_properties(interface_menu PROPERTIES FOLDER "Project")

set_target_properties(interface_menu PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
)

# debugger working dir

target_compile_definitions(interface_menu PRIVATE INTERFACE_EXPORTS)
target_include_directories(interface_menu PUBLIC
    ${RmlUi_SOURCE_DIR}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${stb_SOURCE_DIR}
)

target_link_libraries(interface_menu
     PUBLIC
	 Freetype::Freetype
     RmlUi::RmlUi
     commons
     glm::glm-header-only
	 
)

set_target_properties(interface_menu PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:interface_menu>"
)