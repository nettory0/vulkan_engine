#cmake_minimum_required(VERSION 3.15)
#project(CrossPlatformExample LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  

include(FetchContent)

FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG master
)

set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)

if(TARGET assimp)
    set_target_properties(assimp PROPERTIES FOLDER "External/assimp")
endif()

if(TARGET zlibstatic) 
    set_target_properties(zlibstatic PROPERTIES FOLDER "External/assimp") 
endif()

if(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
    set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "External/assimp")
endif()

if(TARGET unit)
    set_target_properties(unit PROPERTIES FOLDER "External/assimp")
endif()

if(TARGET uninstall)
    set_target_properties(uninstall PROPERTIES FOLDER "External/assimp")
endif()

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)

FetchContent_MakeAvailable(stb)

if(TARGET stb)
    set_target_properties(stb PROPERTIES FOLDER "External")
endif()

add_library(vulkan_engine SHARED
    en_animation_structs.cpp
    en_animation_structs.h
    en_graphics_pipeline.h
    en_camera.cpp
    en_camera.h
    en_context.cpp
    en_context.h
    en_descriptor_manager.cpp
    en_descriptor_manager.h
    en_draw_object.cpp
    en_draw_object.h
    en_core.cpp
    en_core.h
    en_utils.cpp
    en_utils.h
    en_common_utils.h
    en_render_resource.h
    en_render_resource.cpp
    en_vulkan_engine_export.h
    en_node_scene.h
    en_node_scene.cpp
    en_world.h
    en_world.cpp
    en_animation_load.cpp 
    en_animation_load.h 
    en_component_system.h
    en_graphics_pipeline.cpp 
    en_ray_tracing.cpp
    en_ray_tracing.h
    en_models_path.h
    en_compute_system.h 
    en_compute_system.cpp 
    en_scene_component.h 
    en_scene_component.cpp
    en_text_system.h
    en_text_system.cpp
    en_render_system.h
    en_render_system.cpp
    en_input_system.h
    en_input_system.cpp
    en_animation_system.h
    en_animation_system.cpp
    en_select_system.h
    en_select_system.cpp
)

set_target_properties(vulkan_engine PROPERTIES FOLDER "Project")

set_target_properties(vulkan_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
)

target_compile_definitions(vulkan_engine PRIVATE VULKAN_ENGINE_EXPORTS)
target_include_directories(vulkan_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}    
    ${stb_SOURCE_DIR}
)

target_link_libraries(vulkan_engine
    PUBLIC vulkan_struct
    PUBLIC physic_engine
    PUBLIC commons
    PUBLIC text_gen
    PUBLIC asset_loader
    PUBLIC interface_menu
    PRIVATE volk
    PRIVATE VulkanMemoryAllocator
    PRIVATE ${VULKAN_LIBRARY} 
    PRIVATE assimp
)
