# FindTensorRT.cmake - Locate the TensorRT libraries and headers
# This module defines the following variables:
#   TensorRT_FOUND        - True if TensorRT was found
#   TensorRT_INCLUDE_DIRS - Include directories for TensorRT headers
#   TensorRT_LIBRARIES    - Link libraries for TensorRT

# 定义变量，用于存储库的路径和目标信息
set(TensorRT_FOUND FALSE)
set(TensorRT_LIBRARIES)
set(TensorRT_INCLUDE_DIR)

if(TensorRT_ROOT)
    message(STATUS "[message] TensorRT_ROOT: ${TensorRT_ROOT}")
    # 查找TensorRT库文件
    if (WIN32)
        file(GLOB TensorRT_LIBRARY ${TensorRT_ROOT}/lib/*.lib)
    elseif (LINUX)
        file(GLOB TensorRT_LIBRARY ${TensorRT_ROOT}/targets/x86_64-linux-gnu/lib/*.so)
    endif ()
    # 查找TensorRT头文件
    find_path(TensorRT_INCLUDES NAMES
            NvCaffeParser.h
            NvInfer.h
            NvInferConsistency.h
            NvInferConsistencyImpl.h
            NvInferImpl.h
            NvInferLegacyDims.h
            NvInferPlugin.h
            NvInferPluginUtils.h
            NvInferRuntime.h
            NvInferRuntimeBase.h
            NvInferRuntimeCommon.h
            NvInferRuntimePlugin.h
            NvInferSafeRuntime.h
            NvInferVersion.h
            NvOnnxConfig.h
            NvOnnxParser.h
            NvUffParser.h
            NvUtils.h
            PATHS ${TensorRT_ROOT}/include)
    mark_as_advanced(TensorRT_INCLUDES)
endif()

# 检查是否找到库和头文件
if(TensorRT_LIBRARY AND TensorRT_INCLUDES)
    set(TensorRT_LIBRARIES ${TensorRT_LIBRARY} CACHE PATH "TensorRT libraries directory")
    set(TensorRT_INCLUDE_DIR ${TensorRT_INCLUDES} CACHE PATH "TensorRT include directory")
    set(TensorRT_ROOT_DIR ${TensorRT_ROOT} CACHE PATH "TensorRT root directory")
endif()

# 创建导入目标
if(TensorRT_FOUND)
    add_library(TensorRT UNKNOWN IMPORTED)
    set_target_properties(TensorRT PROPERTIES
            IMPORTED_LOCATION "${TensorRT_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${TensorRT_INCLUDE_DIR}"
    )
endif()

# 提供缓存和版本信息
include(FindPackageHandleStandardArgs)
# 这些是必须被找到的变量的列表。如果这些变量中任何一个没有被设置，宏将生成一个错误消息。
find_package_handle_standard_args(TensorRT DEFAULT_MSG TensorRT_LIBRARIES TensorRT_INCLUDE_DIR)

# Print status
if(TensorRT_FOUND)
    message(STATUS "[message] Found TensorRT: ${TensorRT_FOUND}")
    message(STATUS "[message]   - TensorRT root dir: ${TensorRT_ROOT_DIR}")
    message(STATUS "[message]   - TensorRT include dirs: ${TensorRT_INCLUDE_DIR}")
    message(STATUS "[message]   - TensorRT libraries: ${TensorRT_LIBRARIES}")
else()
    message(FATAL_ERROR "Could not find TensorRT")
endif()
