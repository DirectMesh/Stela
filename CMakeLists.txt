cmake_minimum_required(VERSION 3.16)
project(BAGE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

# Detect mobile platforms and configure SDL3 options BEFORE adding SDL3 subdirectory
if(IOS OR TVOS OR ANDROID)
    set(MOBILE_PLATFORM ON)
    set(SDL_POWER OFF CACHE BOOL "Disable power subsystem on mobile" FORCE)
    set(SDL_JOYSTICK OFF CACHE BOOL "Disable joystick subsystem on mobile" FORCE)
    set(SDL_HAPTIC OFF CACHE BOOL "Disable haptic subsystem on mobile (requires joystick)" FORCE)
    set(SDL_SENSOR OFF CACHE BOOL "Disable sensor subsystem on mobile" FORCE)
    set(SDL_HIDAPI OFF CACHE BOOL "Disable HIDAPI on mobile" FORCE)
endif()

# Graphics backend

if(APPLE)
    # Metal (Apple)
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)

    # Vulkan-like imported target
    add_library(Metal::Metal INTERFACE IMPORTED)

    target_link_libraries(Metal::Metal INTERFACE
        ${METAL_FRAMEWORK}
        ${QUARTZCORE_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
        ${COCOA_FRAMEWORK}
    )

else()
    # Vulkan (non-Apple)
    find_package(Vulkan REQUIRED)

    find_program(GLSL_VALIDATOR glslangValidator
        HINTS
            /usr/bin
            /usr/local/bin
            $ENV{VULKAN_SDK}/Bin/
            $ENV{VULKAN_SDK}/Bin32/
    )

    if(GLSL_VALIDATOR)
        file(GLOB_RECURSE GLSL_SOURCE_FILES
            "${PROJECT_SOURCE_DIR}/shaders/*.frag"
            "${PROJECT_SOURCE_DIR}/shaders/*.vert"
            "${PROJECT_SOURCE_DIR}/shaders/*.comp"
        )

        foreach(GLSL ${GLSL_SOURCE_FILES})
            get_filename_component(FILE_NAME ${GLSL} NAME)
            set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")

            add_custom_command(
                OUTPUT ${SPIRV}
                COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
                DEPENDS ${GLSL}
                COMMENT "Compiling ${FILE_NAME} to SPIR-V"
            )

            list(APPEND SPIRV_BINARY_FILES ${SPIRV})
        endforeach()

        add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})
    endif()
endif()

# Subdirectories

add_subdirectory(ThirdParty)
add_subdirectory(BAGE)
add_subdirectory(Runtime)
add_subdirectory(Editor)
