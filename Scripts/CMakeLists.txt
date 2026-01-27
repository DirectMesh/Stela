# Collect common sources
file(GLOB_RECURSE SOURCES
    "Engine/*.cpp"
    "Engine/*.h"
    "User/*.cpp"
    "User/*.h"
    "ScriptsAPI.cpp"
)

add_library(Scripts SHARED ${SOURCES})

set_target_properties(Scripts PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)

target_compile_definitions(Scripts PRIVATE SCRIPTS_EXPORTS)

target_include_directories(Scripts
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_include_directories(Scripts PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Stela/src"
)

target_link_libraries(Scripts PRIVATE Stela)

if(TARGET Shaders)
    add_dependencies(Scripts Shaders)
endif()