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

FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG master
)
FetchContent_MakeAvailable(VulkanMemoryAllocator)

if(TARGET VulkanMemoryAllocator)
    set_target_properties(VulkanMemoryAllocator PROPERTIES FOLDER "External")
endif()

if(TARGET volk)
    set_target_properties(volk PROPERTIES FOLDER "External")
endif()

find_package(Vulkan REQUIRED)

add_library(vulkan_struct SHARED
	vk_struct_builders.cpp
	vk_struct_builders.h
	vk_base.cpp
	vk_base.h
	vk_objects.cpp
	vk_objects.h
	vk_object_template.h
	vk_export.h
	vk_mem_alloc_impl.cpp
)

set_target_properties(vulkan_struct PROPERTIES FOLDER "Project")

set_target_properties(vulkan_struct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
)

target_compile_definitions(vulkan_struct
    PRIVATE VULKAN_STRUCT_EXPORTS
    PUBLIC VK_NO_PROTOTYPES
    # No static Vulkan prototypes (volk) - VMA loads entry points via vkGetInstanceProcAddr/vkGetDeviceProcAddr
    PUBLIC VMA_STATIC_VULKAN_FUNCTIONS=0
    PUBLIC VMA_DYNAMIC_VULKAN_FUNCTIONS=1
)

target_include_directories(vulkan_struct
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${volk_SOURCE_DIR}
)

target_link_libraries(vulkan_struct
    PRIVATE volk
    # Public: vk_mem_alloc.h is included from vk_struct_builders.h
    PUBLIC VulkanMemoryAllocator
    PRIVATE Vulkan::Vulkan
)
