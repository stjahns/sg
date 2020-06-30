cmake_minimum_required(VERSION 3.0)
project(sg)

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
                    src/vendor/assimp/include/
                    src/vendor/glm/
                    src/vendor/imgui/)


file(GLOB VENDORS_SOURCES src/vendor/glad/src/glad.c
	                      src/vendor/imgui/*.cpp)

file(GLOB PROJECT_HEADERS src/sg/*.hpp)

file(GLOB PROJECT_SOURCES src/sg/*.cpp)

file(GLOB PROJECT_SHADERS build/shaders/*.comp
                          build/shaders/*.frag
                          build/shaders/*.geom
                          build/shaders/*.vert)

file(GLOB PROJECT_CONFIGS CMakeLists.txt
                          Readme.md
                         .gitattributes
                         .gitignore
                         .gitmodules)

source_group("Headers" FILES ${PROJECT_HEADERS})
source_group("Shaders" FILES ${PROJECT_SHADERS})
source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Vendors" FILES ${VENDORS_SOURCES})

add_definitions(-DGLFW_INCLUDE_NONE
                -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS}
                               ${PROJECT_SHADERS} ${PROJECT_CONFIGS}
                               ${VENDORS_SOURCES})
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} ${GLAD_LIBRARIES} assimp)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})


add_custom_target(run
    COMMAND sg
    DEPENDS sg
    WORKING_DIRECTORY sg
)