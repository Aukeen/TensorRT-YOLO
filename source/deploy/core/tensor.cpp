
#include "deploy/core/macro.hpp"
#include "deploy/core/tensor.hpp"

namespace deploy {

void Tensor::reallocHost(int64_t bytes) {
    if (hostCap < bytes) {
        CUDA(cudaFreeHost(hostPtr));
        CUDA(cudaMallocHost(&hostPtr, bytes));
        hostCap = bytes;
    }
    hostBytes = bytes;
}

void Tensor::reallocDevice(int64_t bytes) {
    if (deviceCap < bytes) {
        CUDA(cudaFree(devicePtr));
        CUDA(cudaMalloc(&devicePtr, bytes));
        deviceCap = bytes;
    }
    deviceBytes = bytes;
}

Tensor::~Tensor() {
    if (hostPtr != nullptr) {
        CUDA(cudaFreeHost(hostPtr));
    }
    if (devicePtr != nullptr) {
        CUDA(cudaFree(devicePtr));
    }
}

void* Tensor::host(int64_t size) {
    reallocHost(size);
    return hostPtr;
}

void* Tensor::device(int64_t size) {
    reallocDevice(size);
    return devicePtr;
}

TensorInfo::TensorInfo(const char* name, const nvinfer1::Dims& dims, bool input, size_t typeSz, int64_t bytes)
    : name(name),
      dims(dims),
      input(input),
      typeSz(typeSz),
      tensor(Tensor()),
      bytes(bytes) {
}

void TensorInfo::update() {
    bytes = calculateVolume(dims) * typeSz;
}

}  // namespace deploy