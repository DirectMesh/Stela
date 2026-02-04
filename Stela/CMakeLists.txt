# Collect common sources
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

add_library(Stela SHARED ${SOURCES})

set_target_properties(Stela PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)

# Objective-C++ Metal implementation (explicit!)
if(APPLE)
    target_sources(Stela PRIVATE
        src/Render/Metal/Metal.mm
        src/Render/Metal/MetalPrivate.mm
    )
endif()

target_include_directories(Stela
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PUBLIC "/usr/local/share/dotnet/packs/Microsoft.NETCore.App.Host.osx-arm64/10.0.0/runtimes/osx-arm64/native"
)

# Link Vulkan and SDL on non-Apple, SDL only on Apple
if(NOT APPLE)
    target_link_libraries(Stela PRIVATE Vulkan::Vulkan SDL3::SDL3)
else()
    target_link_libraries(Stela PRIVATE
        Metal::Metal
        metal-cpp
        SDL3::SDL3
        "/usr/local/share/dotnet/packs/Microsoft.NETCore.App.Host.osx-arm64/10.0.0/runtimes/osx-arm64/native/libnethost.dylib"
    )
endif()

if(TARGET Shaders)
    add_dependencies(Stela Shaders)
endif()
