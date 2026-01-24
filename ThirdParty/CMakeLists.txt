add_subdirectory(SDL)

# metal-cpp (headers-only)
if(APPLE)
    add_library(metal-cpp INTERFACE)
    target_include_directories(metal-cpp INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp
    )
endif()