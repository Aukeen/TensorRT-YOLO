# 设置项目
cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0146 OLD)
project(segment LANGUAGES CXX CUDA)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加依赖项
find_package(CUDA REQUIRED)
set(CMAKE_CUDA_ARCHITECTURES native)
set(CUDA_PATH ${CUDA_TOOLKIT_ROOT_DIR})

find_package(OpenCV REQUIRED)

# 添加编译选项
option(TENSORRT_PATH "TensorRT Path. Example: /usr/local/tensorrt" "")
if(NOT DEFINED TENSORRT_PATH)
    message(FATAL_ERROR "TensorRT path is not set. Please specify the TensorRT path.")
endif()

option(DEPLOY_PATH "TensorRT-YOLO Project Path." "")
if(NOT DEFINED DEPLOY_PATH)
    message(FATAL_ERROR "TensorRT-YOLO project path is not set. Please specify the TensorRT-YOLO Project path.")
endif()

# 添加可执行文件
add_executable(segment "")

# 包含头文件目录
target_include_directories(segment PRIVATE
    ${CUDA_INCLUDE_DIRS}
    ${TENSORRT_PATH}/include
    ${OpenCV_INCLUDE_DIRS}
    ${DEPLOY_PATH}/include
)

# 链接库目录
target_link_directories(segment PRIVATE
    ${TENSORRT_PATH}/lib
    ${DEPLOY_PATH}/lib
)

# 私有源文件
target_sources(segment PRIVATE segment.cpp)

# 私有编译定义
target_compile_definitions(segment PRIVATE ${CUDA_DEFINITIONS})

# 私有链接库
target_link_libraries(segment PRIVATE
    ${CUDA_cudart_LIBRARY}
    ${OpenCV_LIBS}
    deploy
)

# 根据平台选择不同的 TensorRT 库
if(MSVC AND EXISTS ${TENSORRT_PATH}/lib/nvinfer_10.dll)
    target_link_libraries(segment PRIVATE nvinfer_10 nvinfer_plugin_10 nvonnxparser_10)
else()
    target_link_libraries(segment PRIVATE nvinfer nvinfer_plugin nvonnxparser)
endif()

# 设置输出目录
set_target_properties(segment PROPERTIES OUTPUT_NAME "segment")
if(MSVC)
    set_target_properties(segment PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    )
else()
    set_target_properties(segment PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    )
endif()

# 编译选项
if(MSVC)
    target_compile_options(segment PRIVATE
        $<$<CONFIG:Release>:-O2>
    )
    set_property(TARGET segment PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    target_compile_options(segment PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-O3 -flto=auto>
    )
    target_link_options(segment PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-O3 -flto=auto>
    )
endif()