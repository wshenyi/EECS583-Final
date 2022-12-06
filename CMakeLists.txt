project(CUDAPrefetch)
cmake_minimum_required(VERSION 3.18.0)

find_package(LLVM REQUIRED CONFIG)                        # This will find the shared LLVM build.
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")        # You don't need to change ${LLVM_CMAKE_DIR} since it is already defined.
include(AddLLVM)
add_definitions(${LLVM_DEFINITIONS})                      # You don't need to change ${LLVM_DEFINITIONS} since it is already defined.
include_directories(${LLVM_INCLUDE_DIRS})                 # You don't need to change ${LLVM_INCLUDE_DIRS} since it is already defined.


# Locate the root directory of the CUDA Toolkit.
# cmake 3.18 introduces the CUDAToolkit_LIBRARY_ROOT variable.
# This will update CUDA_HOME to a value discovered by find_package.
cmake_policy(SET CMP0074 NEW) # Use CUDAToolkit_ROOT as a cmake prefix.
find_package(CUDAToolkit REQUIRED)
set(CUDA_LIB "${CUDA_cuda_driver_LIBRARY}")
include_directories("${CUDAToolkit_INCLUDE_DIRS}")
get_filename_component(CUDA_HOME "${CUDAToolkit_BIN_DIR}" DIRECTORY)
find_library(CUDADEVRT_LIB cudadevrt PATH "${CUDAToolkit_LIBRARY_DIR}")
message(STATUS "Using CUDA_HOME: ${CUDA_HOME}")
message(STATUS "Using CUDA_LIB:  ${CUDA_LIB}")
message(STATUS "Using CUDADEVRT_LIB: ${CUDADEVRT_LIB}")
if (("${CUDA_HOME}" STREQUAL "") OR ("${CUDA_LIB}" STREQUAL ""))
    message(FATAL_ERROR "Failed to locate paths to the CUDA toolkit and nvcc.")
endif ()

# Find the nvvm directory in the toolkit.
find_file(LIBNVVM_HOME nvvm PATHS "$ENV{LIBNVVM_HOME}" "${CUDA_HOME}")
message(STATUS "Using LIBNVVM_HOME: ${LIBNVVM_HOME}")

# Find libNVVM and nvvm.h.
# (Linux: nvvm/lib64, windows: nvvm/lib/x64)
find_library(NVVM_LIB nvvm PATHS "${LIBNVVM_HOME}/lib64" "${LIBNVVM_HOME}/lib/x64")
find_file(NVVM_H nvvm.h PATH "${LIBNVVM_HOME}/include")
get_filename_component(NVVM_INCLUDE_DIR ${NVVM_H} DIRECTORY)
include_directories(${NVVM_INCLUDE_DIR})
message(STATUS "Using libnvvm header:      ${NVVM_H}")
message(STATUS "Using libnvvm header path: ${NVVM_INCLUDE_DIR}")
message(STATUS "Using libnvvm library:     ${NVVM_LIB}")

# Find libcudadevrt.
find_library(CUDADEVRT_LIB cudadevrt PATHS "${CUDA_LIB_ROOT}")
message(STATUS "Using CUDADEVRT_LIB: ${CUDADEVRT_LIB}")
if (NOT CUDADEVRT_LIB)
    message(FATAL_ERROR "Failed to locate the path to libcudadevrt.")
endif ()

# Set the rpath to libnvvm.
find_path(LIBNVVM_RPATH lib lib64 PATHS "$ENV{LIBNVVM_HOME}" "${CUDA_HOME}")
get_filename_component(LIBNVVM_RPATH ${NVVM_LIB} DIRECTORY)
set(CMAKE_INSTALL_RPATH "${LIBNVVM_RPATH}")
message(STATUS "Using rpath: ${CMAKE_INSTALL_RPATH}")

add_definitions(-DLIBDEVICE_MAJOR_VERSION=1)
add_definitions(-DLIBDEVICE_MINOR_VERSION=0)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common/include")

add_subdirectory(src)                                     # Add the directory which your pass lives.
