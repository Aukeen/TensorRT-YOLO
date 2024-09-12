cmake_minimum_required(VERSION 3.18)
project(TensorRT-YOLO LANGUAGES C CXX)
# 编译类型
set(CMAKE_BUILD_TYPE RELEASE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")

# ----------------- Import extra module ----------------- #
# 设置CMake模块路径，用于查找额外的CMake模块
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# 导入FunctionsModule(./cmake/FunctionsModule.cmake)模块
include(FunctionsModule)
# ----------------- Import extra module ----------------- #

# ---------------- C/C++ standard setting --------------- #
# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ---------------- C/C++ standard setting --------------- #

include_directories(include)

file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/source/deploy/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/deploy/*/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/deploy/*.cu
        ${CMAKE_CURRENT_SOURCE_DIR}/source/deploy/*/*.cu)

# -------------------- Compile CUDA --------------------- #
include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    # 为整个项目设置目标架构，要在enable_language(CUDA)之前设置
    if (NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
        set(CMAKE_CUDA_ARCHITECTURES 75 80 CACHE STRING "CUDA architectures" FORCE)
    endif()

    # CUDA 语言不是默认启用的，需要手动启用
    # 注意：使用MSVC编译器时，CUDA的版本必须与之匹配，不然会发生找不到CUDA编译器的问题
    enable_language(CUDA)

    # 设置 CUDA 标准
    if(NOT DEFINED CMAKE_CUDA_STANDARD)
        set(CMAKE_CUDA_STANDARD 11)
        set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    endif()
else()
    message(FATAL_ERROR "[ERROR] CUDA compiler not found.")
endif()

# 代码中如果需要使用到 CUDA Toolkit 提供的库，比如：
#   - CUDA 运行时库（如 cudart）
#   - CUDA 驱动程序 API 库（如 cuda）
#   - 其他 CUDA 工具库（如 cuBLAS、cuFFT、cuRAND 等）
# 那么，就需要使用 find_package(CUDAToolkit REQUIRED) 来确保这些库被正确找到并链接到你的目标中；如果只需要编译
# 一些简单的 CUDA 程序，不依赖于 CUDA Toolkit 中的任何特定库，那么仅使用 enable_language(CUDA) 可能就足够了。
if(NOT CUDAToolkit_FOUND)
    message(STATUS "[message] To find CUDAToolkit ......")
    find_package(CUDAToolkit REQUIRED) # or include(FindCUDAToolkit)
endif()
# 包含CUDAToolkit的头文件路径
include_directories(${CUDAToolkit_INCLUDE_DIRS})

add_definitions(-DUSE_CUDA) # 添加宏定义，用于在代码中判断是否使用 CUDA

show_cuda_info()
# -------------------- Compile CUDA --------------------- #

# ---------------------- TensorRT ----------------------- #
set(TensorRT_ROOT "G:/software/TensorRT/TensorRT-10.2.0.19-cuda12.5" CACHE PATH "TensorRT install directory")
if (TensorRT_ROOT)
    find_package(TensorRT REQUIRED)
    include_directories(${TensorRT_INCLUDE_DIR})
endif ()
# ---------------------- TensorRT ----------------------- #

if (NOT BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
else ()
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
endif ()

# ------------------- Link libraries -------------------- #
if (WIN32 OR LINUX)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            # ${OpenCV_LIBS}
            ${CUDAToolkit_LIBRARY_DIR}/*.lib
            ${TensorRT_LIBRARIES})
else ()
    message(FATAL_ERROR "[ERROR] Current platform is no support.")
endif ()
# ------------------- link libraries -------------------- #


# ----------------------- Install ----------------------- #

# ----------------------- Install ----------------------- #
