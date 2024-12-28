cmake_minimum_required(VERSION 3.18)
project(poker-probability CUDA CXX)

# Set configuration types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Set C++ and CUDA standards
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 11)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# Find required packages
find_package(CUDAToolkit REQUIRED)
find_package(Threads REQUIRED)

# Source files
file(GLOB SOURCES
    "src/*.cpp"
    "src/*.cu"
)

file(GLOB HEADERS
    "include/*.h"
    "include/*.hpp"
    "include/*.cuh"
)

# Create executable
add_executable(poker-probability ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(poker-probability PRIVATE
    ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
    ${CMAKE_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(poker-probability PRIVATE
    CUDA::cudart
    Threads::Threads
)

# CUDA specific settings
set_target_properties(poker-probability PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_ARCHITECTURES "60;70;75;86"
)

# Set compiler flags based on platform and configuration
if(MSVC)
    target_compile_options(poker-probability PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:
            $<$<CONFIG:Debug>:/W4 /Od /Zi>
            $<$<CONFIG:Release>:/O2 /Oi>>
        $<$<COMPILE_LANGUAGE:CUDA>:
            $<$<CONFIG:Debug>:-G>
            $<$<CONFIG:Release>:-O3 --use_fast_math>>
    )
else()
    target_compile_options(poker-probability PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:
            $<$<CONFIG:Debug>:-O0 -g3 -Wall -Wextra>
            $<$<CONFIG:Release>:-O3>>
        $<$<COMPILE_LANGUAGE:CUDA>:
            $<$<CONFIG:Debug>:-G>
            $<$<CONFIG:Release>:-O3 --use_fast_math>>
    )
endif()