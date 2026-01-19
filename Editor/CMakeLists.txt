cmake_minimum_required(VERSION 3.16)
project(BAGE_EDITOR)

add_executable(BAGE_EDITOR
    src/main.cpp
)

set_property(TARGET BAGE_EDITOR PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:BAGE_EDITOR>")

target_include_directories(BAGE_EDITOR PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../BAGE/src"
)

# Link BAGE always
target_link_libraries(BAGE_EDITOR PRIVATE BAGE)

target_link_libraries(BAGE_EDITOR PRIVATE SDL3::SDL3)

# Platform-specific bundle settings
if(APPLE AND NOT MOBILE_PLATFORM)
    set_target_properties(BAGE_EDITOR PROPERTIES
        MACOSX_BUNDLE TRUE
        PRODUCT_BUNDLE_IDENTIFIER "net.directmesh.BAGEEditor"
        MACOSX_BUNDLE_BUNDLE_NAME "BAGE Editor"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )
elseif(IOS)
    # Configure Info.plist from template
    set(EXECUTABLE_NAME "BAGE_EDITOR")
    set(PRODUCT_BUNDLE_IDENTIFIER "net.directmesh.BAGEEditor")
    set(PRODUCT_NAME "BAGE Editor")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
    set(IPHONEOS_DEPLOYMENT_TARGET "11.0")
    
    set(INFO_PLIST_PATH "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
        "${INFO_PLIST_PATH}"
        @ONLY
    )
    
    # Get absolute path for Info.plist
    get_filename_component(INFO_PLIST_ABS "${INFO_PLIST_PATH}" ABSOLUTE)
    
    # Copy Info.plist to bundle after build using explicit path
    add_custom_command(TARGET BAGE_EDITOR POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:BAGE_EDITOR>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${INFO_PLIST_ABS}"
        "$<TARGET_FILE_DIR:BAGE_EDITOR>/Info.plist"
        COMMENT "Copying Info.plist to bundle"
    )
    
    set_target_properties(BAGE_EDITOR PROPERTIES
        MACOSX_BUNDLE TRUE
        BUNDLE TRUE
        PRODUCT_BUNDLE_IDENTIFIER "${PRODUCT_BUNDLE_IDENTIFIER}"
        MACOSX_BUNDLE_BUNDLE_NAME "${PRODUCT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${MACOSX_BUNDLE_BUNDLE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${MACOSX_BUNDLE_SHORT_VERSION_STRING}"
        MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST_ABS}"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "${IPHONEOS_DEPLOYMENT_TARGET}"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${PRODUCT_BUNDLE_IDENTIFIER}"
        XCODE_ATTRIBUTE_INFOPLIST_FILE "${INFO_PLIST_ABS}"
        XCODE_ATTRIBUTE_GENERATE_INFOPLIST_FILE "NO"
    )
elseif(TVOS)
    message(STATUS "Configuring for tvOS")
    # Configure Info.plist from template
    set(EXECUTABLE_NAME "BAGE_EDITOR")
    set(PRODUCT_BUNDLE_IDENTIFIER "net.directmesh.BAGEEditorTV")
    set(PRODUCT_NAME "BAGE Editor")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
    set(TVOS_DEPLOYMENT_TARGET "11.0")
    
    # Generate Info.plist in the binary directory
    set(INFO_PLIST_PATH "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
    message(STATUS "Generating Info.plist from ${CMAKE_CURRENT_SOURCE_DIR}/Info-tvOS.plist.in to ${INFO_PLIST_PATH}")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Info-tvOS.plist.in"
        "${INFO_PLIST_PATH}"
        @ONLY
    )
    message(STATUS "Info.plist generated at ${INFO_PLIST_PATH}")
    
    # Get absolute path for Info.plist
    get_filename_component(INFO_PLIST_ABS "${INFO_PLIST_PATH}" ABSOLUTE)
    
    # Get the bundle output directory
    get_target_property(BUNDLE_OUTPUT_DIR BAGE_EDITOR MACOSX_BUNDLE_LOCATION)
    if(NOT BUNDLE_OUTPUT_DIR)
        set(BUNDLE_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()
    
    # Copy Info.plist to bundle after build using explicit path
    add_custom_command(TARGET BAGE_EDITOR POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:BAGE_EDITOR>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${INFO_PLIST_ABS}"
        "$<TARGET_FILE_DIR:BAGE_EDITOR>/Info.plist"
        COMMENT "Copying Info.plist to bundle"
    )
    
    set_target_properties(BAGE_EDITOR PROPERTIES
        MACOSX_BUNDLE TRUE
        BUNDLE TRUE
        PRODUCT_BUNDLE_IDENTIFIER "${PRODUCT_BUNDLE_IDENTIFIER}"
        MACOSX_BUNDLE_BUNDLE_NAME "${PRODUCT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${MACOSX_BUNDLE_BUNDLE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${MACOSX_BUNDLE_SHORT_VERSION_STRING}"
        MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST_ABS}"
        XCODE_ATTRIBUTE_TVOS_DEPLOYMENT_TARGET "${TVOS_DEPLOYMENT_TARGET}"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${PRODUCT_BUNDLE_IDENTIFIER}"
        XCODE_ATTRIBUTE_INFOPLIST_FILE "${INFO_PLIST_ABS}"
        XCODE_ATTRIBUTE_GENERATE_INFOPLIST_FILE "NO"
    )
endif()

set_target_properties(BAGE_EDITOR PROPERTIES OUTPUT_NAME "BAGE_EDITOR")   