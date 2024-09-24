# FindTensorRT-YOLO.cmake - Locate the TensorRT-YOLO libraries and headers
# This module defines the following variables:
#   TensorRT-YOLO_FOUND        - True if TensorRT-YOLO was found
#   TensorRT-YOLO_INCLUDE_DIRS - Include directories for TensorRT-YOLO headers
#   TensorRT-YOLO_LIBRARIES    - Link libraries for TensorRT-YOLO

# 定义变量，用于存储库的路径和目标信息
set(TensorRT-YOLO_FOUND FALSE)
set(TensorRT-YOLO_LIBRARIES)
set(TensorRT-YOLO_INCLUDE_DIR)

if(TensorRT-YOLO_ROOT)
    message(STATUS "[message] TensorRT-YOLO_ROOT: ${TensorRT-YOLO_ROOT}")
    # 查找TensorRT-YOLO库文件
    file(GLOB TensorRT-YOLO_LIBRARY ${TensorRT-YOLO_ROOT}/lib/*.lib)
    # 查找TensorRT-YOLO头文件
    find_path(TensorRT-YOLO_INCLUDES NAMES
            deploy/core/core.hpp
            deploy/core/macro.hpp
            deploy/core/tensor.hpp
            deploy/core/types.hpp
            deploy/utils/utils.hpp
            deploy/vision/cudaWarp.hpp
            deploy/vision/detection.hpp
            deploy/vision/result.hpp
            PATHS ${TensorRT-YOLO_ROOT}/include)
    mark_as_advanced(TensorRT-YOLO_INCLUDES)
endif()

# 检查是否找到库和头文件
if(TensorRT-YOLO_LIBRARY AND TensorRT-YOLO_INCLUDES)
    set(TensorRT-YOLO_LIBRARIES ${TensorRT-YOLO_LIBRARY} CACHE PATH "TensorRT-YOLO libraries directory")
    set(TensorRT-YOLO_INCLUDE_DIR ${TensorRT-YOLO_INCLUDES} CACHE PATH "TensorRT-YOLO include directory")
    set(TensorRT-YOLO_ROOT_DIR ${TensorRT-YOLO_ROOT} CACHE PATH "TensorRT-YOLO root directory")
endif()

# 创建导入目标
if(TensorRT-YOLO_FOUND)
    add_library(TensorRT-YOLO UNKNOWN IMPORTED)
    set_target_properties(TensorRT-YOLO PROPERTIES
            IMPORTED_LOCATION "${TensorRT-YOLO_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${TensorRT-YOLO_INCLUDE_DIR}"
    )
endif()

# 提供缓存和版本信息
include(FindPackageHandleStandardArgs)
# 这些是必须被找到的变量的列表。如果这些变量中任何一个没有被设置，宏将生成一个错误消息。
find_package_handle_standard_args(TensorRT-YOLO DEFAULT_MSG TensorRT-YOLO_LIBRARIES TensorRT-YOLO_INCLUDE_DIR)

# Print status
if(TensorRT-YOLO_FOUND)
    message(STATUS "[message] Found TensorRT-YOLO: ${TensorRT-YOLO_FOUND}")
    message(STATUS "[message]   - TensorRT-YOLO root dir: ${TensorRT-YOLO_ROOT_DIR}")
    message(STATUS "[message]   - TensorRT-YOLO include dirs: ${TensorRT-YOLO_INCLUDE_DIR}")
    message(STATUS "[message]   - TensorRT-YOLO libraries: ${TensorRT-YOLO_LIBRARIES}")
else()
    message(FATAL_ERROR "Could not find TensorRT-YOLO")
endif()
