####################################################################################################
# Compile GLSL shaders into SPIR-V (.spv) files
#
# Usage:
#   compile_shaders(
#       TARGET my_target
#       SHADERS file1.vert file2.frag
#   )
####################################################################################################

function(compile_shaders)
    include(CMakeParseArguments)

    cmake_parse_arguments(
        SHADERS
        ""
        "TARGET"
        "SHADERS"
        ${ARGN}
    )

    if(NOT SHADERS_TARGET)
        message(FATAL_ERROR "compile_shaders: TARGET is required")
    endif()

    if(NOT SHADERS_SHADERS)
        message(FATAL_ERROR "compile_shaders: SHADERS list is empty")
    endif()

    # Find shader compiler
    find_program(GLSLC_EXECUTABLE glslc REQUIRED)

    set(SPIRV_OUTPUTS)

    foreach(SHADER ${SHADERS_SHADERS})
        # Absolute path of shader
        get_filename_component(SHADER_ABS ${SHADER} ABSOLUTE)

        # Generate: <shader>.<ext>.spv
        set(SPIRV_FILE "${SHADER_ABS}.spv")

        add_custom_command(
            OUTPUT ${SPIRV_FILE}
            COMMAND ${GLSLC_EXECUTABLE}
                    -std=450
                    -g
                    ${SHADER_ABS}
                    -o ${SPIRV_FILE}
            DEPENDS ${SHADER_ABS}
            COMMENT "Compiling shader ${SHADER_ABS}"
            VERBATIM
        )

        list(APPEND SPIRV_OUTPUTS ${SPIRV_FILE})
    endforeach()

    add_custom_target(
        Shaders
        DEPENDS ${SPIRV_OUTPUTS}
    )

    add_dependencies(${SHADERS_TARGET} Shaders)
endfunction()
