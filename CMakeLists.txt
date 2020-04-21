project(lexicographic-sort)
cmake_minimum_required(VERSION 2.6)

add_subdirectory(cz)

include_directories(cz/include)
include_directories(tracy)

file(GLOB_RECURSE SRCS src/*.cpp tracy/TracyClient.cpp)
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} cz)
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} dl)

if (TRACY_ENABLE)
  add_compile_definitions(TRACY_ENABLE)
endif()

if (CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror=switch")
endif()
