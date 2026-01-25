# Collect common sources
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

add_library(Stela STATIC ${SOURCES})

# Objective-C++ Metal implementation (explicit!)
if(APPLE)
    target_sources(Stela PRIVATE
        src/Render/Metal/Metal.mm
        src/Render/Metal/MetalPrivate.mm
    )
endif()

target_include_directories(Stela
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Link Vulkan and SDL on non-Apple, SDL only on Apple
if(NOT APPLE)
    target_link_libraries(Stela PRIVATE Vulkan::Vulkan SDL3::SDL3)
else()
    target_link_libraries(Stela PRIVATE
        Metal::Metal
        metal-cpp
        SDL3::SDL3
    )
endif()

if(TARGET Shaders)
    add_dependencies(Stela Shaders)
endif()
