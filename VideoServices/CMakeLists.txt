# Determine platform-specific sokol-shdc binary path (use host platform for build tools)
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(SOKOL_SHDC_PLATFORM "win32")
    set(SOKOL_SHDC_EXT ".exe")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64")
        set(SOKOL_SHDC_PLATFORM "osx_arm64")
    else()
        set(SOKOL_SHDC_PLATFORM "osx")
    endif()
    set(SOKOL_SHDC_EXT "")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64")
        set(SOKOL_SHDC_PLATFORM "linux_arm64")
    else()
        set(SOKOL_SHDC_PLATFORM "linux")
    endif()
    set(SOKOL_SHDC_EXT "")
else()
    message(FATAL_ERROR "Unsupported host platform: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

set(SOKOL_SHDC ${sokol-tools-bin_SOURCE_DIR}/bin/${SOKOL_SHDC_PLATFORM}/sokol-shdc${SOKOL_SHDC_EXT})

# List of shader source files
set(SHADER_FILES
    GL/shaders/background.glsl
    GL/shaders/free_element.glsl
    GL/shaders/wall.glsl
    GL/shaders/water.glsl
    GL/shaders/world.glsl
)

foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME_WE)
    get_filename_component(SHADER_DIR ${SHADER_FILE} DIRECTORY)
    set(SHADER_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_DIR}/${SHADER_NAME}.h")
    add_custom_command(
        OUTPUT ${SHADER_HEADER}
        COMMAND ${SOKOL_SHDC} --input ${SHADER_FILE} --output ${SHADER_DIR}/${SHADER_NAME}.h --slang glsl300es:glsl410
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating ${SHADER_NAME}.h from ${SHADER_FILE}"
        VERBATIM
    )
    list(APPEND SHADER_HEADERS ${SHADER_HEADER})
endforeach()

add_library(VideoServices STATIC
    2DViewport.cpp
    3DViewport.cpp
    3DViewportRendering.cpp
    Bitmap.cpp
    ColorPalette.cpp
    ColorTab.cpp
    Patch.cpp
    SDLSoundServer.cpp
    Sprite.cpp
    VideoBuffer.cpp
    VideoBuffer2DDraw.cpp
    GL/GLRenderer.cpp
    GL/GLViewport.cpp
    GL/GLViewport.h
    GL/sokol_impl.cpp
    ${SHADER_HEADERS}
)

target_compile_definitions(VideoServices PRIVATE
    MR_VIDEO_SERVICES
)

if(NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)
    target_link_libraries(VideoServices PRIVATE OpenGL::GL)
endif()


target_link_libraries(VideoServices PRIVATE ColorTools Util SDL3::SDL3-static glm::glm sokol imgui
)