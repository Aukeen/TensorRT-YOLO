cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(TensorRT-YOLO
        VERSION 4.2.0
        DESCRIPTION "A TensorRT YOLO implementation project"
        HOMEPAGE_URL "https://github.com/Aukeen/TensorRT-YOLO"
        LANGUAGES C CXX)

# 编译选项
set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "Build type")
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install" CACHE PATH "install path" FORCE)
set(HIDDEN_DETAILS ON CACHE BOOL "Hidden details")
set(BUILD_PYTHON_API ON CACHE BOOL "Build Python API")

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

# -------------------- Compile CUDA --------------------- #
include(CheckLanguage)
check_language(CUDA) # CUDA版本与MSVC版本必须匹配，否则会找不到CUDA编译器
if(CMAKE_CUDA_COMPILER)
    # 为整个项目设置目标架构，要在enable_language(CUDA)之前设置
    if (NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
        set(CMAKE_CUDA_ARCHITECTURES 80 CACHE STRING "CUDA architectures" FORCE)
    endif()

    # CUDA 语言不是默认启用的，需要手动启用
    # 注意：使用MSVC编译器时，CUDA的版本必须与之匹配，不然会发生找不到CUDA编译器的问题
    enable_language(CUDA)

    # 设置 CUDA 标准
    if(NOT DEFINED CMAKE_CUDA_STANDARD)
        set(CMAKE_CUDA_STANDARD 17)
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

add_definitions(-DUSE_CUDA) # 添加宏定义，用于在代码中判断是否使用 CUDA

show_cuda_info()
# -------------------- Compile CUDA --------------------- #

# ---------------------- TensorRT ----------------------- #
set(TensorRT_ROOT "" CACHE PATH "TensorRT install directory")
if (TensorRT_ROOT)
    find_package(TensorRT REQUIRED)
    include_directories(${TensorRT_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "[ERROR] TensorRT_ROOT not set.")
endif ()
# ---------------------- TensorRT ----------------------- #

if (BUILD_PYTHON_API)
# ----------------------- Python ------------------------ #
find_package(Python COMPONENTS Interpreter Development REQUIRED)
if (NOT Python_FOUND)
    message(FATAL_ERROR "[ERROR] Python not found.")
else ()
    message(STATUS "[message] Python version: ${Python_VERSION}")
    message(STATUS "[message]   - Python EXECUTABLE: ${Python_EXECUTABLE}")
    message(STATUS "[message]   - Python INCLUDE_DIRS: ${Python_INCLUDE_DIRS}")
    message(STATUS "[message]   - Python LIBRARIES: ${Python_LIBRARIES}")
endif ()
# ----------------------- Python ------------------------ #

# ---------------------- pybind11 ----------------------- #
if(NOT DEFINED pybind11_FOUND)
    set(PYBIND11_PYTHON_VERSION 3.10)
    set(pybind11_DIR "" CACHE PATH "pybind11 install directory")
    find_package(pybind11 CONFIG REQUIRED)
    message(STATUS "pybind11_INCLUDE_DIRS: ${pybind11_INCLUDE_DIRS}") # Directories where pybind11 and python headers are located.
endif()
# ---------------------- pybind11 ----------------------- #
endif () # BUILD_PYTHON_API

# ----------------- Source file include ----------------- #
# 使用 file(GLOB_RECURSE ...) 命令来自动添加所有 .cpp 文件到项目中，
# 可能会不小心包含了 CMake 自动生成的 CMakeCXXCompilerId.cpp 文件
file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/source/deploy/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/deploy/*/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/deploy/*.cu
        ${CMAKE_CURRENT_SOURCE_DIR}/source/deploy/*/*.cu)

add_library(${PROJECT_NAME}_obj OBJECT ${SOURCES})
set_target_properties(${PROJECT_NAME}_obj
        PROPERTIES
        POSITION_INDEPENDENT_CODE 1
)

add_library(${PROJECT_NAME}_static
        STATIC
        $<TARGET_OBJECTS:${PROJECT_NAME}_obj>
)
set_target_properties(${PROJECT_NAME}_static
        PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
)

if (BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME}_shared
            SHARED
            $<TARGET_OBJECTS:${PROJECT_NAME}_obj>
    )
    set_target_properties(${PROJECT_NAME}_shared
            PROPERTIES
            OUTPUT_NAME ${PROJECT_NAME}
    )
endif()

target_include_directories(${PROJECT_NAME}_obj PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if (BUILD_PYTHON_API)
# pybind11
set(PY_PKG_NAME pydeploy)
pybind11_add_module(${PY_PKG_NAME} MODULE $<TARGET_OBJECTS:${PROJECT_NAME}_obj>)
# C++模块用到了第三方库的动态链接库，可能会在使用.pyd文件时遇到：
# ImportError: DLL load failed while importing XXX: 找不到指定的模块。
# 则需要把对应的dll文件复制到.pyd文件所在的地方
target_include_directories(${PY_PKG_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_include_directories(${PROJECT_NAME}_obj PRIVATE
        ${pybind11_INCLUDE_DIRS}) # pybind11是head-only的，因此只需要导入头文件就可以了
endif () # BUILD_PYTHON_API
# ----------------- Source file include ----------------- #

# --------------------- definitions --------------------- #
if (HIDDEN_DETAILS)
    add_definitions(-DHIDDEN_IMPLEMENTATION)
endif ()

if (BUILD_SHARED_LIBS)
    add_definitions(-DENABLE_DEPLOY_BUILDING_DLL)
endif ()
# --------------------- definitions --------------------- #

# ------------------- Link libraries -------------------- #
if (WIN32 OR LINUX)
    # 包含CUDAToolkit的头文件路径
    # 代码里需要CUDA的API所以对象库需要导入相关头文件
    target_include_directories(${PROJECT_NAME}_obj PRIVATE
            ${CUDAToolkit_INCLUDE_DIRS})

    target_link_libraries(${PY_PKG_NAME}
            PRIVATE
            ${CUDAToolkit_LIBRARY_DIR}/*.lib
            ${TensorRT_LIBRARIES}
    )

    target_link_libraries(${PROJECT_NAME}_static
            PRIVATE
            ${CUDAToolkit_LIBRARY_DIR}/*.lib
            ${TensorRT_LIBRARIES}
            ${Python_LIBRARIES})
    if (BUILD_SHARED_LIBS)
    target_link_libraries(${PROJECT_NAME}_shared
            PRIVATE
            ${CUDAToolkit_LIBRARY_DIR}/*.lib
            ${TensorRT_LIBRARIES}
            ${Python_LIBRARIES})
    endif ()
else ()
    message(FATAL_ERROR "[ERROR] Current platform is no support.")
endif ()
# ------------------- link libraries -------------------- #

# ----------------------- Install ----------------------- #
# 1.相关文件的 install 目录
if (BUILD_SHARED_LIBS)
    set(TARGETS_INSTALL ${PROJECT_NAME}_shared ${PROJECT_NAME}_static)
else ()
    set(TARGETS_INSTALL ${PROJECT_NAME}_static)
endif ()

install(TARGETS ${TARGETS_INSTALL}
        EXPORT ${PROJECT_NAME}Config
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

# 2.导出公共头文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

# 3.Generate and install CMake package configuration files
install(EXPORT ${PROJECT_NAME}Config
        FILE ${PROJECT_NAME}Config.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION lib/cmake/${PROJECT_NAME}
)

# 4.版本
include(CMakePackageConfigHelpers)
# 用于记录所需包的版本和兼容性
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION lib/cmake/${PROJECT_NAME}
)
# ----------------------- Install ----------------------- #
