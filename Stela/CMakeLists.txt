# Collect common sources
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

add_library(Stela SHARED ${SOURCES})

set_target_properties(Stela PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
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
# Try to locate nethost header and library in common dotnet locations (all platforms).
file(GLOB DOTNET_HOST_HDRS
    "/usr/share/dotnet/sdk/*/Sdks/Microsoft.NET.Sdk/targets/native/nethost.h"
    "/usr/local/share/dotnet/sdk/*/Sdks/Microsoft.NET.Sdk/targets/native/nethost.h"
    "/usr/share/dotnet/sdk/*/Sdks/Microsoft.NET.Sdk/targets/native/hostfxr.h"
    "/usr/local/share/dotnet/sdk/*/Sdks/Microsoft.NET.Sdk/targets/native/hostfxr.h"
    "/usr/share/dotnet/packs/*/*/runtimes/*/native/nethost.h"
    "/usr/local/share/dotnet/packs/*/*/runtimes/*/native/nethost.h"
    "/usr/share/dotnet/packs/*/*/runtimes/*/native/hostfxr.h"
    "/usr/local/share/dotnet/packs/*/*/runtimes/*/native/hostfxr.h"
    "/usr/share/dotnet/packs/*/*/runtimes/*/native/coreclr_delegates.h"
    "C:/Program Files/dotnet/sdk/*/Sdks/Microsoft.NET.Sdk/targets/native/nethost.h"
    "C:/Program Files/dotnet/packs/*/*/runtimes/*/native/nethost.h"
    "C:/Program Files/dotnet/packs/*/*/runtimes/*/native/hostfxr.h"
    "C:/Program Files/dotnet/packs/*/*/runtimes/*/native/coreclr_delegates.h"
)
if(DOTNET_HOST_HDRS)
    set(_host_dirs "")
    foreach(_hdr IN LISTS DOTNET_HOST_HDRS)
        get_filename_component(_dir ${_hdr} DIRECTORY)
        list(FIND _host_dirs ${_dir} _found_index)
        if(_found_index EQUAL -1)
            list(APPEND _host_dirs ${_dir})
            message(STATUS "Found dotnet host header: ${_hdr}")
            message(STATUS "Adding host include dir: ${_dir}")
        endif()
    endforeach()
    target_include_directories(Stela PUBLIC ${_host_dirs})
endif()

# Also add any pack 'native' dirs to includes (covers coreclr_delegates.h location)
file(GLOB DOTNET_NATIVE_DIRS
    "/usr/share/dotnet/packs/*/*/runtimes/*/native"
    "/usr/local/share/dotnet/packs/*/*/runtimes/*/native"
    "C:/Program Files/dotnet/packs/*/*/runtimes/*/native"
)
if(DOTNET_NATIVE_DIRS)
    foreach(_d IN LISTS DOTNET_NATIVE_DIRS)
        message(STATUS "Adding dotnet native dir to includes: ${_d}")
    endforeach()
    target_include_directories(Stela PUBLIC ${DOTNET_NATIVE_DIRS})
endif()

# Known distro pack path where coreclr_delegates.h often lives; add explicitly if present
set(_distro_coreclr_dir "/usr/share/dotnet/packs/Microsoft.NETCore.App.Host.arch-x64/9.0.11/runtimes/arch-x64/native")
if(EXISTS ${_distro_coreclr_dir})
    message(STATUS "Adding distro coreclr native dir: ${_distro_coreclr_dir}")
    target_include_directories(Stela PUBLIC ${_distro_coreclr_dir})
endif()

# Also search for coreclr_delegates.h and hostfxr.h in packs (some distros place them there)
file(GLOB CORECLR_DELEGATES_HDRS
    "/usr/share/dotnet/packs/*/runtimes/*/native/coreclr_delegates.h"
    "/usr/local/share/dotnet/packs/*/runtimes/*/native/coreclr_delegates.h"
)
if(CORECLR_DELEGATES_HDRS)
    list(GET CORECLR_DELEGATES_HDRS 0 _hdr)
    get_filename_component(_dir ${_hdr} DIRECTORY)
    message(STATUS "Found coreclr_delegates.h: ${_hdr}")
    target_include_directories(Stela PUBLIC ${_dir})
endif()

file(GLOB HOSTFXR_HDRS
    "/usr/share/dotnet/packs/*/runtimes/*/native/hostfxr.h"
    "/usr/local/share/dotnet/packs/*/runtimes/*/native/hostfxr.h"
    "C:/Program Files/dotnet/packs/*/runtimes/*/native/hostfxr.h"
)
if(HOSTFXR_HDRS)
    list(GET HOSTFXR_HDRS 0 _hdr)
    get_filename_component(_dir ${_hdr} DIRECTORY)
    message(STATUS "Found hostfxr.h: ${_hdr}")
    target_include_directories(Stela PUBLIC ${_dir})
endif()

file(GLOB DOTNET_NETHOST_LIBS
    "/usr/share/dotnet/packs/*/runtimes/*/native/libnethost.*"
    "/usr/local/share/dotnet/packs/*/runtimes/*/native/libnethost.*"
)

# Prefer shared libnethost (.so) when available, fallback to any libnethost.*
file(GLOB DOTNET_NETHOST_LIBS_SO
    "/usr/share/dotnet/packs/*/runtimes/*/native/libnethost.so"
    "/usr/local/share/dotnet/packs/*/runtimes/*/native/libnethost.so"
)
file(GLOB DOTNET_NETHOST_LIBS_ANY
    "/usr/share/dotnet/packs/*/*/runtimes/*/native/libnethost.*"
    "/usr/local/share/dotnet/packs/*/*/runtimes/*/native/libnethost.*"
    "C:/Program Files/dotnet/packs/*/*/runtimes/*/native/nethost.lib"
)

set(NETHOST_LIB "")
if(DOTNET_NETHOST_LIBS_SO)
    list(GET DOTNET_NETHOST_LIBS_SO 0 NETHOST_LIB)
elseif(DOTNET_NETHOST_LIBS_ANY)
    # Filter candidates based on architecture
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_arch_tag "x64")
    else()
        set(_arch_tag "x86")
    endif()

    foreach(_lib IN LISTS DOTNET_NETHOST_LIBS_ANY)
        if(_lib MATCHES "${_arch_tag}")
            set(NETHOST_LIB ${_lib})
            break()
        endif()
    endforeach()

    # Fallback to first if no match found
    if(NOT NETHOST_LIB)
        list(GET DOTNET_NETHOST_LIBS_ANY 0 NETHOST_LIB)
    endif()
endif()

if(NETHOST_LIB)
    message(STATUS "Using libnethost: ${NETHOST_LIB}")
else()
    message(STATUS "libnethost not found by glob; checking known distro paths")
    set(_cand_paths
        "/usr/share/dotnet/packs/Microsoft.NETCore.App.Host.arch-x64/9.0.11/runtimes/arch-x64/native/libnethost.so"
        "/usr/share/dotnet/packs/Microsoft.NETCore.App.Host.arch-x64/9.0.11/runtimes/arch-x64/native/libnethost.a"
    )
    foreach(_p IN LISTS _cand_paths)
        if(EXISTS ${_p})
            set(NETHOST_LIB ${_p})
            message(STATUS "Found libnethost candidate: ${_p}")
            break()
        endif()
    endforeach()
endif()

if(NOT APPLE)
    target_link_libraries(Stela PUBLIC Vulkan::Vulkan PRIVATE SDL3::SDL3 ${NETHOST_LIB})
else()
    # Try to locate nethost header and library in common dotnet locations.
    file(GLOB DOTNET_NETHOST_HDRS
        "/usr/share/dotnet/sdk/*/Sdks/Microsoft.NET.Sdk/targets/native/nethost.h"
        "/usr/local/share/dotnet/sdk/*/Sdks/Microsoft.NET.Sdk/targets/native/nethost.h"
        "/usr/share/dotnet/packs/*/runtimes/*/native/nethost.h"
        "/usr/local/share/dotnet/packs/*/runtimes/*/native/nethost.h"
    )
    if(DOTNET_NETHOST_HDRS)
        list(GET DOTNET_NETHOST_HDRS 0 NETHOST_HDR)
        get_filename_component(NETHOST_DIR ${NETHOST_HDR} DIRECTORY)
        target_include_directories(Stela PUBLIC ${NETHOST_DIR})
    endif()

    file(GLOB DOTNET_NETHOST_LIBS
        "/usr/share/dotnet/packs/*/runtimes/*/native/libnethost.*"
        "/usr/local/share/dotnet/packs/*/runtimes/*/native/libnethost.*"
    )
    if(DOTNET_NETHOST_LIBS)
        list(GET DOTNET_NETHOST_LIBS 0 NETHOST_LIB)
    else()
        # Fallback to existing hardcoded macOS path if present
        set(NETHOST_LIB "/usr/local/share/dotnet/packs/Microsoft.NETCore.App.Host.osx-arm64/10.0.0/runtimes/osx-arm64/native/libnethost.dylib")
    endif()

    target_link_libraries(Stela PRIVATE
        Metal::Metal
        metal-cpp
        SDL3::SDL3
        ${NETHOST_LIB}
    )
endif()

if(TARGET Shaders)
    add_dependencies(Stela Shaders)
endif()
