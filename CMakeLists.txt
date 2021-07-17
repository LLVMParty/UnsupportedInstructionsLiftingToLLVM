cmake_minimum_required(VERSION 3.20)

project(uil)

# Set the C++ standard

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Set the compilation options

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Include LLVM

include(LLVM)

# Register Zydis

option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)
add_subdirectory("deps/zydis")

# Add the project sources and includes

set(SOURCES
  src/main.cpp)

set(INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${SOURCES})

# Link the dependencies and includes

target_link_libraries(${PROJECT_NAME} PRIVATE Zydis)
target_link_libraries(${PROJECT_NAME} PRIVATE LLVM)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})