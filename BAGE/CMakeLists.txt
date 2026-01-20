file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_library(BAGE STATIC ${SOURCES})

target_include_directories(BAGE PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Link Vulkan and SDL on non-Apple, SDL only on Apple
if(NOT APPLE)
    target_link_libraries(BAGE PRIVATE Vulkan::Vulkan SDL3::SDL3)
else()
    target_link_libraries(BAGE PRIVATE SDL3::SDL3)
endif()

if(TARGET Shaders)
    add_dependencies(BAGE Shaders)
endif()