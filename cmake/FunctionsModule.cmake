include(CMakeParseArguments)

#if(POLICY CMP0146)
#    # ------------------------------ 设置CMP0146策略 ------------------------------ #
#    # 从CMake 3.15版本开始，FindCUDA模块不再是CMake默认模块，而是被移动到了CMake的额外模块中
#    # (./cmake-3.xx.x-windows-x86_64/share/cmake-3.xx/Modules/FindCUDA.cmake)
#    # 如果你的项目依赖于这个模块，你需要手动检查并包含它。# 这行命令将CMP0146策略设置为NEW，这意味
#    # 着你接受了新的行为（即FindCUDA模块被移除）。添加这个策略设置后，CMake将不会再显示这个警告。
#    # cmake_policy(SET CMP0146 NEW)
#    # cmake_policy(SET CMP0146 OLD)
#    # ---------------------------------------------------------------------------- #
#    cmake_policy(SET CMP0146 OLD)
#endif()

function(show_cuda_info)
    message(STATUS "[message] CUDA Info:")

    message(STATUS "[message]   - CUDAToolkit_FOUND: ${CUDAToolkit_FOUND}")
    #A boolean specifying whether or not the CUDA Toolkit was found.

    message(STATUS "[message]   - CUDAToolkit_VERSION: ${CUDAToolkit_VERSION}")
    #The exact version of the CUDA Toolkit found (as reported by nvcc --version, version.txt, or version.json).

    message(STATUS "[message]   - CUDAToolkit_VERSION_MAJOR: ${CUDAToolkit_VERSION_MAJOR}")
    #The major version of the CUDA Toolkit.

    message(STATUS "[message]   - CUDAToolkit_VERSION_MINOR: ${CUDAToolkit_VERSION_MINOR}")
    #The minor version of the CUDA Toolkit.

    message(STATUS "[message]   - CUDAToolkit_VERSION_PATCH: ${CUDAToolkit_VERSION_PATCH}")
    #The patch version of the CUDA Toolkit.

    message(STATUS "[message]   - CUDAToolkit_BIN_DIR: ${CUDAToolkit_BIN_DIR}")
    #The path to the CUDA Toolkit library directory that contains the CUDA executable nvcc.

    message(STATUS "[message]   - CUDAToolkit_INCLUDE_DIRS: ${CUDAToolkit_INCLUDE_DIRS}") # equal CUDA_INCLUDE_DIRS
    #List of paths to all the CUDA Toolkit folders containing header files required to compile a project linking against CUDA.

    message(STATUS "[message]   - CUDAToolkit_LIBRARY_DIR: ${CUDAToolkit_LIBRARY_DIR}")
    #The path to the CUDA Toolkit library directory that contains the CUDA Runtime library cudart.

    message(STATUS "[message]   - CUDAToolkit_LIBRARY_ROOT: ${CUDAToolkit_LIBRARY_ROOT}") # equal CUDA_TOOLKIT_ROOT_DIR
    #New in version 3.18.
    #The path to the CUDA Toolkit directory containing the nvvm directory and either version.txt or version.json.

    message(STATUS "[message]   - CUDAToolkit_TARGET_DIR: ${CUDAToolkit_TARGET_DIR}")
    #The path to the CUDA Toolkit directory including the target architecture when cross-compiling. When not
    #cross-compiling this will be equivalent to the parent directory of CUDAToolkit_BIN_DIR.

    message(STATUS "[message]   - CUDAToolkit_NVCC_EXECUTABLE: ${CUDAToolkit_NVCC_EXECUTABLE}")
    #The path to the NVIDIA CUDA compiler nvcc. Note that this path may not be the same as CMAKE_CUDA_COMPILER.
    #nvcc must be found to determine the CUDA Toolkit version as well as determining other features of the Toolkit.
    #This variable is set for the convenience of modules that depend on this one.

    message(STATUS "[message]   - CUDA_cudart_static_LIBRARY: ${CUDA_cudart_static_LIBRARY}")
    message(STATUS "[message]   - CUDA_cudart_LIBRARY: ${CUDA_cudart_LIBRARY}")
    message(STATUS "[message]   - CUDA_nvrtc_LIBRARY: ${CUDA_nvrtc_LIBRARY}")
    message(STATUS "[message]   - CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES: ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}")
    message(STATUS "[message]   - CMAKE_CUDA_COMPILER: ${CMAKE_CUDA_COMPILER}")
    message(STATUS "[message]   - CMAKE_CUDA_COMPILER_ID: ${CMAKE_CUDA_COMPILER_ID}")
    message(STATUS "[message]   - CMAKE_CUDA_COMPILER_VERSION: ${CMAKE_CUDA_COMPILER_VERSION}")
    message(STATUS "[message]   - CMAKE_CUDA_FLAGS: ${CMAKE_CUDA_FLAGS}")
    message(STATUS "[message]   - CMAKE_CUDA_FLAGS_DEBUG: ${CMAKE_CUDA_FLAGS_DEBUG}")
    message(STATUS "[message]   - CMAKE_CUDA_FLAGS_RELEASE: ${CMAKE_CUDA_FLAGS_RELEASE}")
    message(STATUS "[message]   - CMAKE_CUDA_STANDARD: ${CMAKE_CUDA_STANDARD}")
    message(STATUS "[message]   - CMAKE_CUDA_STANDARD_REQUIRED: ${CMAKE_CUDA_STANDARD_REQUIRED}")
endfunction()