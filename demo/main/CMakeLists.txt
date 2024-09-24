cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(Main_OpenCV
        VERSION 0.0.1
        DESCRIPTION "A TensorRT YOLO implementation project"
        HOMEPAGE_URL "https://github.com/Aukeen/TensorRT-YOLO"
        LANGUAGES C CXX)

# 编译类型
set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "Build type")
set(BUILD_EXECUTABLES ON CACHE BOOL "Build EXECUTABLES")
#set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install CACHE PATH "Install path")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ---------------- C/C++ standard setting --------------- #
# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ---------------- C/C++ standard setting --------------- #

# ----------------- CUDAToolkit setting ----------------- #
if(NOT CUDAToolkit_FOUND)
    message(STATUS "[message] To find CUDAToolkit ......")
    find_package(CUDAToolkit REQUIRED) # or include(FindCUDAToolkit)
endif()
# ----------------- CUDAToolkit setting ----------------- #

# ----------------------- OpenCV ------------------------ #
set(OpenCV_DIR "G:/software/OpenCV/build/opencv-4.10.0-cuda12.4-msvc2019-qt6.5.3/install")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
# ----------------------- OpenCV ------------------------ #

# ---------------------- TensorRT ----------------------- #
set(TensorRT_ROOT "G:/software/TensorRT/TensorRT-10.2.0.19-cuda12.5" CACHE PATH "TensorRT install directory")
if (TensorRT_ROOT)
    find_package(TensorRT REQUIRED)
else ()
    message(FATAL_ERROR "[ERROR] TensorRT_ROOT not set.")
endif ()
# ---------------------- TensorRT ----------------------- #

# -------------------- TensorRT_YOLO -------------------- #
set(TensorRT-YOLO_ROOT "G:/github/TensorRT-YOLO/cmake-build-release-msvs2022x64/install")
find_package(TensorRT-YOLO REQUIRED)
if (NOT TensorRT-YOLO_FOUND)
    message(FATAL_ERROR "TensorRT-YOLO not found")
else()
    message(STATUS "TensorRT-YOLO found")
    message(STATUS "TensorRT-YOLO_INCLUDE_DIRS: ${TensorRT-YOLO_INCLUDE_DIR}")
endif ()
# -------------------- TensorRT_YOLO -------------------- #

# ----------------- Source file include ----------------- #
file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

if (NOT BUILD_EXECUTABLES)
    if (NOT BUILD_SHARED_LIBS)
        add_library(${PROJECT_NAME} STATIC ${SOURCES})
    else ()
        add_library(${PROJECT_NAME} SHARED ${SOURCES})
    endif ()
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
# ----------------- Source file include ----------------- #

# ------------------- Link libraries -------------------- #
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CUDAToolkit_INCLUDE_DIRS}
        ${TensorRT_INCLUDE_DIR}
        ${TensorRT-YOLO_INCLUDE_DIR}
)

if (WIN32 OR LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            ${OpenCV_LIBS}
            ${CUDAToolkit_LIBRARY_DIR}/*.lib
            ${TensorRT_LIBRARIES}
            ${TensorRT-YOLO_LIBRARIES})
    message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
    message(STATUS "TensorRT-YOLO_LIBRARIES: ${TensorRT-YOLO_LIBRARIES}")
else ()
    message(FATAL_ERROR "[ERROR] Current platform is no support.")
endif ()
# ------------------- link libraries -------------------- #
