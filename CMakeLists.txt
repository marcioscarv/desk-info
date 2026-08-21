cmake_minimum_required(VERSION 3.20)

project(DeskInfo VERSION 2.2.0 LANGUAGES CXX RC)

add_executable(DeskInfo WIN32
    src/app.cpp
    src/main.cpp
    src/system_info.cpp
    src/template_renderer.cpp
    src/DeskInfo.rc
)

set_property(TARGET DeskInfo PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_compile_features(DeskInfo PRIVATE cxx_std_20)
target_compile_definitions(DeskInfo PRIVATE
    UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX _WIN32_WINNT=0x0A00
)
target_link_libraries(DeskInfo PRIVATE iphlpapi ws2_32)

if(MSVC)
    target_compile_options(DeskInfo PRIVATE /W4 /permissive- /utf-8)
    target_link_options(DeskInfo PRIVATE /OPT:REF /OPT:ICF)
endif()

foreach(runtime_file IN ITEMS config.cfg readme.txt)
    add_custom_command(TARGET DeskInfo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/${runtime_file}"
            "$<TARGET_FILE_DIR:DeskInfo>/${runtime_file}"
    )
endforeach()

install(TARGETS DeskInfo RUNTIME DESTINATION .)
install(FILES config.cfg readme.txt DESTINATION .)
