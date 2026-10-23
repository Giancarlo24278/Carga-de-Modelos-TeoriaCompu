cmake_minimum_required(VERSION 3.16)
project(NaveRenderer LANGUAGES CXX)
option(HEADLESS "Generar PPM sin ventana ni SDL" OFF)
add_executable(nave main.cpp)
target_compile_features(nave PRIVATE cxx_std_17)
if(HEADLESS)
    target_compile_definitions(nave PRIVATE HEADLESS)
else()
    set(SDL2_NO_MWINDOWS TRUE)
    find_package(SDL2 CONFIG REQUIRED)
    target_link_libraries(nave PRIVATE SDL2::SDL2)
    if(WIN32)
        # La DLL debe estar junto al ejecutable para abrirlo con doble clic.
        get_target_property(_sdl2_type SDL2::SDL2 TYPE)
        if(_sdl2_type STREQUAL "SHARED_LIBRARY")
            add_custom_command(TARGET nave POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:nave>
                VERBATIM)
        endif()
    endif()
endif()
set(NAVE_RUNTIME_DLL_DIR "" CACHE PATH "Carpeta de las DLL del compilador portatil")
if(WIN32 AND NAVE_RUNTIME_DLL_DIR)
    foreach(_runtime IN ITEMS libc++.dll libunwind.dll libwinpthread-1.dll)
        if(EXISTS "${NAVE_RUNTIME_DLL_DIR}/${_runtime}")
            add_custom_command(TARGET nave POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${NAVE_RUNTIME_DLL_DIR}/${_runtime}" $<TARGET_FILE_DIR:nave>
                VERBATIM)
        endif()
    endforeach()
endif()
configure_file(Nave.obj Nave.obj COPYONLY)
