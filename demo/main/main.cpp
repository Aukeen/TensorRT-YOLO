//
// Created by 22175 on 2024/9/21.
//
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <deploy/utils/utils.hpp>
#include <deploy/vision/detection.hpp>
#include <deploy/vision/result.hpp>

#if defined(_WIN32) || defined( _MSC_VER)
// For loadLibrary
// Needed so that the max/min definitions in windows.h do not conflict with std::max/min.
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#else
#include <dlfcn.h>
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
            //ASSERT(
                static_cast<bool>(FreeLibrary(static_cast<HMODULE>(mHandle)))
            //)
            ;
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



int main(){
    auto t = deploy::CpuTimer();
    t.start();
    cv::Mat img = cv::Mat::zeros(512,512,CV_8UC3);
    cv::rectangle(img, cv::Point(10, 10), cv::Point(100, 100), cv::Scalar(255, 0, 0), 3, cv::LINE_AA);
    cv::imshow("test",img);
    cv::waitKey(0);
    std::string f   = "a";
    auto        ret = deploy::Box{0};
    try {
        auto det = deploy::DeployDet(f, false);
    } catch (std::exception e) {
        std::cout << e.what() << std::endl;
    }
    std::cout << "Hello, World!" << std::endl;
    t.stop();
    std::cout << t.microseconds() << "ms" << std::endl;
    return 0;
}
