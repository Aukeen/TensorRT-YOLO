//
// Created by 22175 on 2024/9/24.
//
#include "deploy/vision/result.hpp"

namespace deploy {

Image::Image(void *rgbPtr, int width, int height) : rgbPtr(rgbPtr), width(width), height(height) {
    if (width < 0 || height < 0) {
        throw std::invalid_argument("Width and height must be non-negative");
    }
}

}  // namespace deploy
