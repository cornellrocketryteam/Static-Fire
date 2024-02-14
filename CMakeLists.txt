cmake_minimum_required(VERSION 3.27)

project(solid C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(LJM_LIBRARY "/usr/local/lib/libLabJackM.dylib")
include_directories("/usr/local/include/")

# Add compiler flags and definitions
add_compile_options(
    -Wall
    -Wno-format          
    -Wno-unused-function
)
# Add source files from src/
set(SOURCES
    src/main.cpp
)

# Add all executables
add_executable(${PROJECT_NAME}
    ${SOURCES}
)

add_subdirectory(lib)

target_link_libraries(${PROJECT_NAME} ${LJM_LIBRARY})
