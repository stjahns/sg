cmake_minimum_required(VERSION 3.10)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(googletest)
add_executable(UnitTests test.cpp)

include_directories(${PROJECT_SOURCE_DIR}/src/sg/anim)

target_link_libraries(UnitTests gtest_main assimp sg_anim)
