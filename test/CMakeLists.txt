cmake_minimum_required(VERSION 3.10)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(googletest)

file(GLOB TEST_SOURCES *.cpp)
add_executable(UnitTests ${TEST_SOURCES})

include_directories(${PROJECT_SOURCE_DIR}/src/sg/anim)

target_link_libraries(UnitTests gtest_main assimp sg_anim)
