set(SHADER_BIN "${CMAKE_BINARY_DIR}/bin/shaders")

set(SHADERS
    anim_model.frag
    anim_model.vert
    shader.frag
    shader.vert
    shader1.frag
    shader1.vert
    Compute/shader.comp
    2d/shader1.frag
    2d/shader1.vert
    ui/shader_interface.frag
    ui/shader_interface.vert
    RayTracing/shader.rchit
    RayTracing/shader.rgen
    RayTracing/shader.rmiss
    RayTracing/shader.shadow.rmiss
)

find_program(GLSL_VALIDATOR glslangValidator HINTS "$ENV{VULKAN_SDK}/Bin")
if(NOT GLSL_VALIDATOR)
    message(FATAL_ERROR "glslangValidator (Vulkan SDK).")
endif()

set(COMPILED_SHADERS "")
set(SHADER_SOURCES "")

foreach(SHADER ${SHADERS})
    set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}")
    set(SPV "${SHADER_BIN}/${SHADER}.spv")

    get_filename_component(SPV_DIR ${SPV} DIRECTORY)
    add_custom_command(
        OUTPUT ${SPV}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SPV_DIR}
        COMMAND ${GLSL_VALIDATOR} -V ${SRC} --target-env vulkan1.4 -o ${SPV}
        DEPENDS ${SRC}
        COMMENT " ${SHADER}"
    )

    list(APPEND COMPILED_SHADERS ${SPV})
    list(APPEND SHADER_SOURCES  ${SRC})
endforeach()

add_custom_target(compile_shaders ALL
    DEPENDS ${COMPILED_SHADERS}
    SOURCES ${SHADER_SOURCES}
)

set_source_files_properties(${SHADER_SOURCES} PROPERTIES
    VS_TOOL_OVERRIDE "None"
    HEADER_FILE_ONLY TRUE
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Shaders"
    FILES ${SHADER_SOURCES}
)

source_group("SPIR-V" FILES ${COMPILED_SHADERS})

set_target_properties(compile_shaders PROPERTIES FOLDER "Project")