add_library(BAGE
    main.cpp
)


set_property(TARGET BAGE PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:BAGE>")

target_include_directories(BAGE PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if(TARGET Vulkan::Vulkan)
    target_link_libraries(BAGE Vulkan::Vulkan SDL3::SDL3)
else()
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(VK REQUIRED vulkan)
    endif()
    if(TARGET Vulkan::Vulkan)
        target_link_libraries(BAGE Vulkan::Vulkan SDL3::SDL3)
    elseif(VK_FOUND)
        target_include_directories(BAGE PUBLIC ${VK_INCLUDE_DIRS})
        target_link_libraries(BAGE ${VK_LIBRARIES} SDL3::SDL3)
    else()
        message(WARNING "Vulkan target not found; linking SDL3 only")
        target_link_libraries(BAGE SDL3::SDL3)
    endif()
endif()

if(TARGET Shaders)
    add_dependencies(BAGE Shaders)
endif()