# Collect common sources
file(GLOB_RECURSE SOURCES
    "Engine/*.cpp"
    "Engine/*.h"
    "User/*.cpp"
    "User/*.h"
    "ScriptsAPI.cpp"
)

add_library(Scripts SHARED ${SOURCES})

# When Scripts folder is next to executable (e.g. bin/Scripts/), build into Scripts/bin
# so the runtime can find the library there; root CMake copies it to exe dir as well.
set(SCRIPTS_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Scripts/bin")
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(SCRIPTS_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/bin/Scripts/bin")
endif()
set_target_properties(Scripts PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${SCRIPTS_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${SCRIPTS_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${SCRIPTS_OUTPUT_DIR}"
)
# Multi-config generators (Visual Studio) append Debug/Release by default; force same dir so Editor hot reload finds the built DLL
if(CMAKE_CONFIGURATION_TYPES)
    set_target_properties(Scripts PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SCRIPTS_OUTPUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SCRIPTS_OUTPUT_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${SCRIPTS_OUTPUT_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${SCRIPTS_OUTPUT_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${SCRIPTS_OUTPUT_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${SCRIPTS_OUTPUT_DIR}"
    )
endif()

target_compile_definitions(Scripts PRIVATE SCRIPTS_EXPORTS)

target_include_directories(Scripts
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_include_directories(Scripts PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Stela/src"
)

target_link_libraries(Scripts PRIVATE Stela SDL3::SDL3)

# When Scripts folder is next to exe, copy built library to root executable path
if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    add_custom_command(TARGET Scripts POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Scripts>
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_FILE_NAME:Scripts>"
        COMMENT "Copying Scripts library to executable directory"
    )
endif()

if(TARGET Shaders)
    add_dependencies(Scripts Shaders)
endif()