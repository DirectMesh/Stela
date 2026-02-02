add_subdirectory(SDL)

# metal-cpp (headers-only)
if(APPLE)
    add_library(metal-cpp INTERFACE)
    target_include_directories(metal-cpp INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp
    )
endif()

# ImGui (submodule: ThirdParty/imgui)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
    set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

    # Core ImGui sources
    set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
    )

    add_library(imgui STATIC ${IMGUI_SOURCES})
    target_include_directories(imgui PUBLIC ${IMGUI_DIR})
    set_target_properties(imgui PROPERTIES POSITION_INDEPENDENT_CODE ON)

    # Backends: SDL + Vulkan (desktop) or Metal (Apple)
    set(IMGUI_BACKEND_SOURCES)
    if(TARGET SDL3::SDL3)
        list(APPEND IMGUI_BACKEND_SOURCES ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp)
    endif()

    if(APPLE)
        # Metal backend (Objective-C++)
        if(EXISTS ${IMGUI_DIR}/backends/imgui_impl_metal.mm)
            list(APPEND IMGUI_BACKEND_SOURCES ${IMGUI_DIR}/backends/imgui_impl_metal.mm)
        endif()
        # QuartzCore for CAMetalLayer, Metal framework will be linked later by consumers
    else()
        # Prefer Vulkan backend when Vulkan is available, otherwise use OpenGL3 if present
        find_package(Vulkan QUIET)
        if(Vulkan_FOUND AND EXISTS ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp)
            list(APPEND IMGUI_BACKEND_SOURCES ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp)
        elseif(EXISTS ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
            list(APPEND IMGUI_BACKEND_SOURCES ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
        endif()
    endif()

    if(IMGUI_BACKEND_SOURCES)
        add_library(imgui_backends STATIC ${IMGUI_BACKEND_SOURCES})
        target_include_directories(imgui_backends PUBLIC
            ${IMGUI_DIR}
            ${IMGUI_DIR}/backends
        )
        target_link_libraries(imgui_backends PUBLIC imgui)
        if(TARGET SDL3::SDL3)
            target_link_libraries(imgui_backends PUBLIC SDL3::SDL3)
        endif()
        if(DEFINED Vulkan_FOUND AND Vulkan_FOUND)
            if(TARGET Vulkan::Vulkan)
                target_link_libraries(imgui_backends PUBLIC Vulkan::Vulkan)
            else()
                target_link_libraries(imgui_backends PUBLIC ${Vulkan_LIBRARIES})
            endif()
        endif()

        if(APPLE)
            find_library(METAL_FRAMEWORK Metal)
            find_library(QUARTZCORE_FRAMEWORK QuartzCore)
            if(METAL_FRAMEWORK)
                target_link_libraries(imgui_backends PUBLIC "-framework Metal")
            endif()
            if(QUARTZCORE_FRAMEWORK)
                target_link_libraries(imgui_backends PUBLIC "-framework QuartzCore")
            endif()
            target_compile_options(imgui_backends PRIVATE -fobjc-arc)
            set_target_properties(imgui_backends PROPERTIES
                POSITION_INDEPENDENT_CODE ON
                PREFIX ""
            )
        endif()

        message(STATUS "Configured ImGui targets: imgui, imgui_backends")
    else()
        message(STATUS "Configured ImGui core only (no backend sources detected)")
    endif()
endif()