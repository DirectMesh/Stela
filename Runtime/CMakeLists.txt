add_executable(Runtime
    src/main.cpp
)


set_property(TARGET Runtime PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Runtime>")

target_include_directories(Runtime PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if(TARGET Vulkan::Vulkan)
    target_link_libraries(Runtime PRIVATE Vulkan::Vulkan SDL3::SDL3)
else()
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(VK REQUIRED vulkan)
    endif()
    if(TARGET Vulkan::Vulkan)
        target_link_libraries(Runtime PRIVATE Vulkan::Vulkan SDL3::SDL3)
    elseif(VK_FOUND)
        target_include_directories(Runtime PUBLIC ${VK_INCLUDE_DIRS})
        target_link_libraries(Runtime PRIVATE ${VK_LIBRARIES} SDL3::SDL3)
    else()
        message(WARNING "Vulkan target not found; linking SDL3 only")
        target_link_libraries(Runtime PRIVATE SDL3::SDL3)
    endif()
endif()

if(TARGET Shaders)
    add_dependencies(Runtime Shaders)
endif()

if(TARGET BAGE)
    if(WIN32)
        add_custom_command(TARGET Runtime POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:BAGE> $<TARGET_FILE_DIR:Runtime>)
    endif()
endif()

if(TARGET BAGE)
    target_link_libraries(Runtime PRIVATE BAGE)
endif()