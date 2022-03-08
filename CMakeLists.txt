# CMakeLists.txt for building Nelson and its dependencies
#
# This file was modified for the purpose of building this project and is based
# on Juliette Focaut and Doug Bink's GLFW-CMake-starter project for simplifying
# building OpenGL projects using GLFW as a windowing library.
#
# GLFW-CMake-starter found at https://github.com/juliettef/GLFW-CMake-starter;

cmake_minimum_required( VERSION 3.1 )

project( Nelson )

set(CMAKE_CXX_STANDARD 11)

find_package( OpenGL REQUIRED )

include_directories( ${OPENGL_INCLUDE_DIRS} )

set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )

add_subdirectory( vendor/glfw )
add_subdirectory( vendor/glew/build/cmake )

if( MSVC )
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
endif()

include_directories(vendor/glew/include)

file(GLOB GLM_SRC
    "vendor/glm/*.hpp"
    "vendor/glm/ext/*.hpp"
    "vendor/glm/gtc/*.hpp"
    "vendor/glm/gtc/*.hpp"
)

file(GLOB SOIL2_SRC
     "vendor/SOIL2/*.h"
     "vendor/SOIL2/*.c"
)

file(GLOB IMGUI_SRC
    "vendor/imgui/imgui.h"
    "vendor/imgui/*.cpp"
    "vendor/imgui/backends/imgui_impl_glfw.h"
    "vendor/imgui/backends/imgui_impl_glfw.cpp"
    "vendor/imgui/backends/imgui_impl_opengl3.h"
    "vendor/imgui/backends/imgui_impl_opengl3.cpp"
)

file(GLOB CORE_SRC
    "src/*.cpp"
    "src/*.h"
    "src/*.c"
)
     
add_executable( ${PROJECT_NAME} WIN32 ${GLEW_INC} ${CORE_SRC} ${SOIL2_SRC} ${IMGUI_SRC} ${SHADER_RES} ${IMAGE_RES})
target_link_libraries( ${PROJECT_NAME} ${OPENGL_LIBRARIES} glew glfw )
if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.1.0") 
        message( "\n\t[ WARNING ]\n\n\tCMake version lower than 3.1.\n\n\t - Please update CMake and rerun; OR\n\t - Manually set 'Nelson as StartUp Project in Visual Studio.\n" )
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME} )
    endif()
endif()
