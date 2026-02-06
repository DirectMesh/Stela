# Collect common sources
file(GLOB_RECURSE SOURCES
    "Engine/*.cpp"
    "Engine/*.h"
    "User/*.cpp"
    "User/*.h"
    "ScriptsAPI.cpp"
    "DotNetHost.cpp"
    "DotNetHost.h"
)

message(STATUS "Scripts sources: ${SOURCES}")

# When Scripts folder is next to executable (e.g. bin/Scripts/), build into Scripts/bin
# so the runtime can find the library there; root CMake copies it to exe dir as well.
set(SCRIPTS_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Scripts/bin")
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(SCRIPTS_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/bin/Scripts/bin")
endif()

# Add ScriptLoader build target
add_custom_target(ScriptLoaderBuild ALL
    COMMAND dotnet build "${CMAKE_CURRENT_SOURCE_DIR}/DotNet/ScriptLoader/ScriptLoader.csproj" -p:TargetFramework=net9.0
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SCRIPTS_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/DotNet/ScriptLoader/bin/Debug/net9.0/ScriptLoader.dll"
        "${SCRIPTS_OUTPUT_DIR}/ScriptLoader.dll"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Stela_EDITOR>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/DotNet/ScriptLoader/bin/Debug/net9.0/ScriptLoader.dll"
        "$<TARGET_FILE_DIR:Stela_EDITOR>/ScriptLoader.dll"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Stela_RUNTIME>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/DotNet/ScriptLoader/bin/Debug/net9.0/ScriptLoader.dll"
        "$<TARGET_FILE_DIR:Stela_RUNTIME>/ScriptLoader.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/DotNet/ScriptLoader/bin/Debug/net9.0/ScriptLoader.runtimeconfig.json"
        "${SCRIPTS_OUTPUT_DIR}/ScriptLoader.runtimeconfig.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/DotNet/ScriptLoader/bin/Debug/net9.0/ScriptLoader.runtimeconfig.json"
        "$<TARGET_FILE_DIR:Stela_EDITOR>/ScriptLoader.runtimeconfig.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/DotNet/ScriptLoader/bin/Debug/net9.0/ScriptLoader.runtimeconfig.json"
        "$<TARGET_FILE_DIR:Stela_RUNTIME>/ScriptLoader.runtimeconfig.json"
    COMMENT "Building C# ScriptLoader (forcing TargetFramework=net9.0)"
)

# Add C# UserScripts build target
add_custom_target(UserScriptsBuild ALL
    COMMAND dotnet build "${CMAKE_CURRENT_SOURCE_DIR}/DotNet/UserScripts/UserScripts.csproj" -p:TargetFramework=net9.0
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SCRIPTS_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/DotNet/UserScripts/bin/Debug/net9.0/UserScripts.dll"
        "${SCRIPTS_OUTPUT_DIR}/UserScripts.dll"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Stela_EDITOR>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/DotNet/UserScripts/bin/Debug/net9.0/UserScripts.dll"
        "$<TARGET_FILE_DIR:Stela_EDITOR>/UserScripts.dll"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Stela_RUNTIME>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/DotNet/UserScripts/bin/Debug/net9.0/UserScripts.dll"
        "$<TARGET_FILE_DIR:Stela_RUNTIME>/UserScripts.dll"
    # We do NOT need UserScripts.runtimeconfig.json for the loader, but it doesn't hurt.
    # The Loader runs using its own runtimeconfig.
    COMMENT "Building C# UserScripts"
)

add_library(Scripts SHARED ${SOURCES})

add_dependencies(Scripts ScriptLoaderBuild UserScriptsBuild)

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

# Try to locate nethost header and library in common dotnet locations.
# This covers SDK-native headers and runtime packs on Linux/macOS.
file(GLOB DOTNET_HOST_HDRS
    "/usr/share/dotnet/sdk/*/Sdks/Microsoft.NET.Sdk/targets/native/nethost.h"
    "/usr/local/share/dotnet/sdk/*/Sdks/Microsoft.NET.Sdk/targets/native/nethost.h"
    "/usr/share/dotnet/sdk/*/Sdks/Microsoft.NET.Sdk/targets/native/hostfxr.h"
    "/usr/local/share/dotnet/sdk/*/Sdks/Microsoft.NET.Sdk/targets/native/hostfxr.h"
    "/usr/share/dotnet/packs/*/runtimes/*/native/nethost.h"
    "/usr/local/share/dotnet/packs/*/runtimes/*/native/nethost.h"
    "/usr/share/dotnet/packs/*/runtimes/*/native/hostfxr.h"
    "/usr/local/share/dotnet/packs/*/runtimes/*/native/hostfxr.h"
    "/usr/share/dotnet/packs/*/runtimes/*/native/coreclr_delegates.h"
    "C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-*/*/runtimes/*/native/nethost.h"
    "C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-*/*/runtimes/*/native/hostfxr.h"
    "C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-*/*/runtimes/*/native/coreclr_delegates.h"
)
if(DOTNET_HOST_HDRS)
    set(_host_dirs "")
    foreach(_hdr IN LISTS DOTNET_HOST_HDRS)
        get_filename_component(_dir ${_hdr} DIRECTORY)
        list(FIND _host_dirs ${_dir} _found_index)
        if(_found_index EQUAL -1)
            list(APPEND _host_dirs ${_dir})
            message(STATUS "Found dotnet host header (scripts): ${_hdr}")
            message(STATUS "Adding host include dir (scripts): ${_dir}")
        endif()
    endforeach()
    target_include_directories(Scripts PUBLIC ${_host_dirs})
endif()

file(GLOB DOTNET_NATIVE_DIRS
    "/usr/share/dotnet/packs/*/runtimes/*/native"
    "/usr/local/share/dotnet/packs/*/runtimes/*/native"
    "C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-*/*/runtimes/*/native"
)
if(DOTNET_NATIVE_DIRS)
    foreach(_d IN LISTS DOTNET_NATIVE_DIRS)
        message(STATUS "Adding dotnet native dir to includes (scripts): ${_d}")
    endforeach()
    target_include_directories(Scripts PUBLIC ${DOTNET_NATIVE_DIRS})
endif()

# Known distro pack path where coreclr_delegates.h often lives; add explicitly if present
set(_distro_coreclr_dir "/usr/share/dotnet/packs/Microsoft.NETCore.App.Host.arch-x64/9.0.11/runtimes/arch-x64/native")
if(EXISTS ${_distro_coreclr_dir})
    message(STATUS "Adding distro coreclr native dir (scripts): ${_distro_coreclr_dir}")
    target_include_directories(Scripts PUBLIC ${_distro_coreclr_dir})
endif()

file(GLOB CORECLR_DELEGATES_HDRS
    "/usr/share/dotnet/packs/*/runtimes/*/native/coreclr_delegates.h"
    "/usr/local/share/dotnet/packs/*/runtimes/*/native/coreclr_delegates.h"
    "C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-*/*/runtimes/*/native/coreclr_delegates.h"
)
if(CORECLR_DELEGATES_HDRS)
    list(GET CORECLR_DELEGATES_HDRS 0 _hdr)
    get_filename_component(_dir ${_hdr} DIRECTORY)
    message(STATUS "Found coreclr_delegates.h (scripts): ${_hdr}")
    target_include_directories(Scripts PUBLIC ${_dir})
endif()

file(GLOB HOSTFXR_HDRS
    "/usr/share/dotnet/packs/*/runtimes/*/native/hostfxr.h"
    "/usr/local/share/dotnet/packs/*/runtimes/*/native/hostfxr.h"
    "C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-*/*/runtimes/*/native/hostfxr.h"
)
if(HOSTFXR_HDRS)
    list(GET HOSTFXR_HDRS 0 _hdr)
    get_filename_component(_dir ${_hdr} DIRECTORY)
    message(STATUS "Found hostfxr.h (scripts): ${_hdr}")
    target_include_directories(Scripts PUBLIC ${_dir})
endif()

file(GLOB DOTNET_NETHOST_LIBS_ANY
    "/usr/share/dotnet/packs/*/runtimes/*/native/libnethost.*"
    "/usr/local/share/dotnet/packs/*/runtimes/*/native/libnethost.*"
    "C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-*/*/runtimes/*/native/nethost.lib"
)

# Architecture-aware library selection
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_arch_tag "x64")
else()
    set(_arch_tag "x86")
endif()

set(NETHOST_LIB "")
if(DOTNET_NETHOST_LIBS_ANY)
    foreach(_lib IN LISTS DOTNET_NETHOST_LIBS_ANY)
        if(_lib MATCHES "${_arch_tag}")
            set(NETHOST_LIB ${_lib})
            break()
        endif()
    endforeach()
    
    # Fallback if no arch match found (e.g. non-windows or weird naming)
    if(NOT NETHOST_LIB)
        list(GET DOTNET_NETHOST_LIBS_ANY 0 NETHOST_LIB)
    endif()
endif()

if(NOT NETHOST_LIB AND APPLE)
    set(_fallback "/usr/local/share/dotnet/packs/Microsoft.NETCore.App.Host.osx-arm64/10.0.0/runtimes/osx-arm64/native/libnethost.dylib")
    if(EXISTS "${_fallback}")
        set(NETHOST_LIB "${_fallback}")
    endif()
endif()

if(NETHOST_LIB)
    message(STATUS "Using libnethost (scripts): ${NETHOST_LIB}")
    
    # On Windows, we need nethost.dll next to the executable
    if(WIN32)
        get_filename_component(_nethost_dir ${NETHOST_LIB} DIRECTORY)
        set(_possible_dll "${_nethost_dir}/nethost.dll")
        if(EXISTS "${_possible_dll}")
            set(NETHOST_DLL "${_possible_dll}")
            message(STATUS "Found nethost.dll: ${NETHOST_DLL}")
        endif()
    endif()
else()
    message(WARNING "Could not find libnethost/nethost.lib! Scripts module may fail to link.")
endif()

target_link_libraries(Scripts PRIVATE Stela SDL3::SDL3 ${NETHOST_LIB})

# When Scripts folder is next to exe, copy built library to root executable path
if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    add_custom_command(TARGET Scripts POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Scripts>
            "$<TARGET_FILE_DIR:Stela_EDITOR>/$<TARGET_FILE_NAME:Scripts>"
        COMMENT "Copying Scripts library to executable directory"
    )
    
    if(NETHOST_DLL)
        add_custom_command(TARGET Scripts POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${NETHOST_DLL}"
                "$<TARGET_FILE_DIR:Stela_EDITOR>/nethost.dll"
            COMMENT "Copying nethost.dll to executable directory"
        )
    endif()
endif()

if(TARGET Shaders)
    add_dependencies(Scripts Shaders)
endif()