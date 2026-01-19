add_executable(Runtime
    src/main.cpp
)


set_property(TARGET Runtime PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Runtime>")

target_include_directories(Runtime PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


target_link_libraries(Runtime PRIVATE SDL3::SDL3 BAGE)

target_include_directories(Runtime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../BAGE/src)