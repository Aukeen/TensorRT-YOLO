//
// Created by 22175 on 2024/9/21.
//
#include <iostream>
#include <string>
#include <random>
#include <memory>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
//#include <deploy/utils/utils.hpp>
//#include <deploy/vision/detection.hpp>
//#include <deploy/vision/result.hpp>

#if defined(_WIN32) || defined( _MSC_VER)
// For loadLibrary
// Needed so that the max/min definitions in windows.h do not conflict with std::max/min.
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#else
#include <dlfcn.h>
#endif

#ifdef NDEBUG
#define ASSERT(condition) condition
#else
#define ASSERT(condition) condition
#endif

#include <functional>

//! Platform-agnostic wrapper around dynamic libraries.
class DynamicLibrary
{
public:
    explicit DynamicLibrary(std::string const& name)
        : mLibName{name}
    {
#if defined(_WIN32)
        mHandle = LoadLibraryA(name.c_str());
#else // defined(_WIN32)
        int32_t flags{RTLD_LAZY};
#if ENABLE_ASAN
        // https://github.com/google/sanitizers/issues/89
        // asan doesn't handle module unloading correctly and there are no plans on doing
        // so. In order to get proper stack traces, don't delete the shared library on
        // close so that asan can resolve the symbols correctly.
        flags |= RTLD_NODELETE;
#endif // ENABLE_ASAN

        mHandle = dlopen(name.c_str(), flags);
#endif // defined(_WIN32)

        if (mHandle == nullptr)
        {
            std::string errorStr{};
#if !defined(_WIN32)
            errorStr = std::string{" due to "} + std::string{dlerror()};
#endif
            throw std::runtime_error("Unable to open library: " + name + errorStr);
        }
    }

    DynamicLibrary(DynamicLibrary const&) = delete;
    DynamicLibrary(DynamicLibrary const&&) = delete;

    //!
    //! Retrieve a function symbol from the loaded library.
    //!
    //! \return the loaded symbol on success
    //! \throw std::invalid_argument if loading the symbol failed.
    //!
    template <typename Signature>
    std::function<Signature> symbolAddress(char const* name)
    {
        if (mHandle == nullptr)
        {
            throw std::runtime_error("Handle to library is nullptr.");
        }
        void* ret;
#if defined(_MSC_VER)
        ret = static_cast<void*>(GetProcAddress(static_cast<HMODULE>(mHandle), name));
#else
        ret = dlsym(mHandle, name);
#endif
        if (ret == nullptr)
        {
            std::string const kERROR_MSG(mLibName + ": error loading symbol: " + std::string(name));
            throw std::invalid_argument(kERROR_MSG);
        }
        return reinterpret_cast<Signature*>(ret);
    }

    ~DynamicLibrary()
    {
        try
        {
#if defined(_WIN32)
            ASSERT(static_cast<bool>(FreeLibrary(static_cast<HMODULE>(mHandle))));
#else
            ASSERT(dlclose(mHandle) == 0);
#endif
        }
        catch (...)
        {
            std::cout << "Unable to close library: " << mLibName << std::endl;
        }
    }

private:
    std::string mLibName{}; //!< Name of the DynamicLibrary
    void* mHandle{};        //!< Handle to the DynamicLibrary
};


namespace fs = std::filesystem;

// Get image files in a directory
std::vector<std::string> getImagesInDirectory(const std::string& folderPath) {
    std::vector<std::string> imageFiles;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        const auto extension = entry.path().extension().string();
        if (fs::is_regular_file(entry) && (extension == ".jpg" || extension == ".png" || extension == ".jpeg" || extension == ".bmp")) {
            imageFiles.push_back(entry.path().string());
        }
    }
    return imageFiles;
}

// Create directory
void createDirectory(const std::string& path) {
    if (!fs::exists(path) && !fs::create_directories(path)) {
        throw std::runtime_error("Failed to create output directory: " + path);
    } else if (!fs::is_directory(path)) {
        throw std::runtime_error("Output path exists but is not a directory: " + path);
    }
}

// Generate label and color pairs
std::vector<std::pair<std::string, cv::Scalar>> generateLabelColorPairs(const std::string& labelFile) {
    std::cout << "Failed to open labels file: " << labelFile << std::endl;
    std::ifstream                                   file(labelFile);
    if (!file.is_open()) {
        std::cout << "Failed to open labels file: " << labelFile << std::endl;
        throw std::runtime_error("Failed to open labels file: " + labelFile);
    }

    std::vector<std::pair<std::string, cv::Scalar>> labelColorPairs;

    auto generateRandomColor = []() {
        std::random_device                 rd;
        std::mt19937                       gen(rd());
        std::uniform_int_distribution<int> dis(0, 255);
        return cv::Scalar(dis(gen), dis(gen), dis(gen));
    };

    std::string label;
    while (std::getline(file, label)) {
        labelColorPairs.emplace_back(label, generateRandomColor());
    }
    return labelColorPairs;
}

void test_opencvCUDA(cv::Mat& image) {
    try
    {
        cv::cuda::GpuMat dst, src;
        src.upload(image);
        // 转成灰度图像
        cv::cuda::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
        cv::cuda::threshold(src, dst, 120, 255, cv::THRESH_BINARY);

        cv::Mat result_host;
        dst.download(result_host);

        cv::imshow("Result", result_host);
        cv::waitKey();
    }
    catch(const cv::Exception& ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;
    }
}

int main(){
    cv::Mat image = cv::imread("E:/N/mmexport1694359809268.jpg");
    cv::resize(image, image, cv::Size(640, 640));
    test_opencvCUDA(image);
    cv::destroyAllWindows();
    return 0;
}
