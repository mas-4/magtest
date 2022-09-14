cmake_minimum_required(VERSION 3.24)
project(magma_proj)

set(CMAKE_CXX_STANDARD 14)
set(GPU_TARGET Ampere)

set(MAGMA ${CMAKE_CURRENT_SOURCE_DIR}/tpc/magma-2.6.2)

if (UNIX)
    set(MKL_DIR "/opt/intel/oneapi/mkl/2022.1.0/lib/cmake/mkl/")
endif (UNIX)

add_compile_options(
        "-Wno-deprecated-declarations"
        "$<$<CONFIG:RELEASE>:-O3>"
)

add_subdirectory(${MAGMA})

add_executable(magma_proj main.cpp)

find_package(CUDA REQUIRED)
find_package(MKL CONFIG REQUIRED)
find_package(LAPACK REQUIRED)

target_include_directories(magma_proj PRIVATE ${CUDA_INCLUDE_DIRS})
target_link_libraries(magma_proj PRIVATE ${CUDA_LIBRARIES})
target_link_libraries(magma_proj PRIVATE MKL::MKL)
target_link_libraries(magma_proj PRIVATE ${LAPACK_LIBRARIES})
target_include_directories(magma_proj PRIVATE ${MAGMA}/include)
target_link_libraries(magma_proj PRIVATE magma)
