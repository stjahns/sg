cmake_minimum_required(VERSION 3.10)
project(sg_demo)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(src/vendor/glfw)
add_subdirectory(src/vendor/assimp)

set(BUILD_STATIC_LIB ON)
set(BUILD_ASSIMP_TOOLS  ON)
set(ASSIMP_BUILD_STATIC_LIB ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

include_directories(src/vendor/
                    src/vendor/glad/include/
                    src/vendor/glfw/include/
                    src/vendor/assimp
                    src/vendor/glm/
                    src/vendor/imgui/)


file(GLOB VENDORS_SOURCES src/vendor/glad/src/glad.c
	                      src/vendor/imgui/*.cpp)

file(GLOB PROJECT_HEADERS src/sg/*.hpp)

file(GLOB PROJECT_SOURCES src/main.cpp)

file(GLOB PROJECT_CONFIGS CMakeLists.txt
                          Readme.md
                         .gitattributes
                         .gitignore
                         .gitmodules)

add_definitions(-DGLFW_INCLUDE_NONE
                -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

file(GLOB SG_LIB_SOURCES src/sg/*.cpp)
add_library(sg_lib STATIC ${SG_LIB_SOURCES})
target_link_libraries(sg_lib assimp)
target_include_directories(sg_lib PUBLIC src/vender/assimp)

file(GLOB ANIM_SOURCES src/sg/anim/*.cpp)
add_library(sg_anim STATIC ${ANIM_SOURCES})
target_link_libraries(sg_anim assimp)
target_include_directories(sg_anim PUBLIC src/vender/assimp)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS} ${PROJECT_CONFIGS} ${VENDORS_SOURCES})

target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} ${GLAD_LIBRARIES} assimp sg_anim sg_lib)

add_subdirectory(test)