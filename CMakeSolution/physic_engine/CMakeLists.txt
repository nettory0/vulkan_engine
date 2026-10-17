#cmake_minimum_required(VERSION 3.15)
#project(CrossPlatformExample LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

include(FetchContent)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    bullet3
    GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
    GIT_TAG master 
)

set(BUILD_BULLET3 OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET_SOFTBODY OFF CACHE BOOL "" FORCE)
set(BUILD_OPENCL OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(bullet3)

foreach(t
    BulletSoftBody
    Bullet3OpenCL_clew
    Bullet3Geometry
    BulletDynamics
    BulletCollision
    LinearMath
    Bullet3Common
    BulletInverseDynamics
    BulletInverseDynamicsUtils
)
    if(TARGET ${t})
        set_target_properties(${t} PROPERTIES FOLDER "External/Bullet")
    endif()
endforeach()



FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glm)

if(TARGET glm)
    set_target_properties(glm PROPERTIES FOLDER "External")
endif()

add_library(physic_engine SHARED
    pe_physic_engine.cpp
    pe_physic_engine.h
	pe_physic_export.h
    pe_engine_core.h 
    pe_engine_impl.cpp)

 set_target_properties(physic_engine PROPERTIES FOLDER "Project")

set_target_properties(physic_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/"
)

target_compile_definitions(physic_engine PRIVATE PHYSIC_EXPORT PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_include_directories(physic_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


target_include_directories(physic_engine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  
    PUBLIC ${glm_SOURCE_DIR}
    PRIVATE ${bullet3_SOURCE_DIR}/src 
       
)

target_link_libraries(physic_engine PRIVATE
    glm::glm-header-only
    BulletDynamics
    BulletCollision
    LinearMath
    PUBLIC commons
)
