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

// Create output directory
void createOutputDirectory(const std::string& outputPath) {
    if (!fs::exists(outputPath) && !fs::create_directories(outputPath)) {
        throw std::runtime_error("Failed to create output directory: " + outputPath);
    } else if (!fs::is_directory(outputPath)) {
        throw std::runtime_error("Output path exists but is not a directory: " + outputPath);
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

// Converts a bounding box with a given angle to its four corner points
std::vector<cv::Point> xyxyr2xyxyxyxy(const deploy::Box& box) {
    // Calculate the cosine and sine of the angle
    float cos_value = std::cos(box.theta);
    float sin_value = std::sin(box.theta);

    // Calculate the center coordinates of the box
    float center_x = (box.left + box.right) * 0.5f;
    float center_y = (box.top + box.bottom) * 0.5f;

    // Calculate the half width and half height of the box
    float half_width  = (box.right - box.left) * 0.5f;
    float half_height = (box.bottom - box.top) * 0.5f;

    // Calculate the rotated corner vectors
    float vec_x1 = half_width * cos_value;
    float vec_y1 = half_width * sin_value;
    float vec_x2 = half_height * sin_value;
    float vec_y2 = half_height * cos_value;

    // Return the four corners of the rotated rectangle
    return {
        cv::Point(center_x + vec_x1 - vec_x2, center_y + vec_y1 + vec_y2),
        cv::Point(center_x + vec_x1 + vec_x2, center_y + vec_y1 - vec_y2),
        cv::Point(center_x - vec_x1 + vec_x2, center_y - vec_y1 - vec_y2),
        cv::Point(center_x - vec_x1 - vec_x2, center_y - vec_y1 + vec_y2)};
}

// Visualize detection results
void visualize(cv::Mat& image, const deploy::DetectionResult& result, const std::vector<std::pair<std::string, cv::Scalar>>& labelColorPairs, bool is_obb) {
    for (size_t i = 0; i < result.num; ++i) {
        const auto& box       = result.boxes[i];
        int         cls       = result.classes[i];
        float       score     = result.scores[i];
        const auto& label     = labelColorPairs[cls].first;
        const auto& color     = labelColorPairs[cls].second;
        std::string labelText = label + " " + cv::format("%.2f", score);

        // Draw rectangle and label
        int      baseLine;
        cv::Size labelSize = cv::getTextSize(labelText, cv::FONT_HERSHEY_SIMPLEX, 0.6, 1, &baseLine);

        if (is_obb) {
            auto corners = xyxyr2xyxyxyxy(box);
            cv::polylines(image, {corners}, true, color, 2, cv::LINE_AA);
            cv::rectangle(image, cv::Point(corners[0].x, corners[0].y - labelSize.height), cv::Point(corners[0].x + labelSize.width, corners[0].y), color, -1);
            cv::putText(image, labelText, corners[0], cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
        } else {
            cv::rectangle(image, cv::Point(box.left, box.top), cv::Point(box.right, box.bottom), color, 2, cv::LINE_AA);
            cv::rectangle(image, cv::Point(box.left, box.top - labelSize.height), cv::Point(box.left + labelSize.width, box.top), color, -1);
            cv::putText(image, labelText, cv::Point(box.left, box.top), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
        }
    }
}


int main(){
//    auto t = deploy::CpuTimer();
//    t.start();
//    cv::Mat img = cv::Mat::zeros(512,512,CV_8UC3);
//    cv::rectangle(img, cv::Point(10, 10), cv::Point(100, 100), cv::Scalar(255, 0, 0), 3, cv::LINE_AA);
//    cv::imshow("test",img);
//    cv::waitKey(0);
//    std::string f   = "a";
//    auto        ret = deploy::Box{0};
//    try {
//        auto det = deploy::DeployDet(f, false);
//    } catch (std::exception e) {
//        std::cout << e.what() << std::endl;
//    }
    std::cerr << "Hello, World!" << std::endl;
//    t.stop();
//    std::cout << t.microseconds() << "ms" << std::endl;

    int         mode         = 0;
    bool        useCudaGraph = false;
    std::string enginePath   = "";
    std::string inputPath    = "";
    std::string outputPath   = "";
    std::string labelPath    = "";

    if (mode != 0 && mode != 1) {
        std::cerr << "Error: "
                  << "Invalid mode: " + std::to_string(mode) + ". Please use 0 for Detection, 1 for OBB." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<std::pair<std::string, cv::Scalar>> labels;
    if (!outputPath.empty()) {
        labels = generateLabelColorPairs(labelPath);
        createOutputDirectory(outputPath);
    }

    bool is_obb = (mode == 1);

    std::shared_ptr<deploy::BaseDet> model;
    try{
        if (useCudaGraph) {
            model = std::make_shared<deploy::DeployCGDet>(enginePath, is_obb);
        } else {
            model = std::make_shared<deploy::DeployDet>(enginePath, is_obb);
        }
    } catch (std::exception e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (fs::is_regular_file(inputPath)) {
        cv::Mat cvimage = cv::imread(inputPath, cv::IMREAD_COLOR);
        cv::cvtColor(cvimage, cvimage, cv::COLOR_BGR2RGB);
        deploy::Image image(cvimage.data, cvimage.cols, cvimage.rows);
        auto          result = model->predict(image);
        if (!outputPath.empty()) {
            cv::cvtColor(cvimage, cvimage, cv::COLOR_RGB2BGR);
            visualize(cvimage, result, labels, is_obb);
            cv::imwrite(outputPath + "/" + fs::path(inputPath).filename().string(), cvimage);
        }
    } else {
        auto imageFiles = getImagesInDirectory(inputPath);
        if (imageFiles.empty()) {
            throw std::runtime_error("No image files found in the directory: " + inputPath);
        }

        int              count     = 0;
        const size_t     batchSize = model->batch;
        deploy::GpuTimer gpuTimer;
        deploy::CpuTimer cpuTimer;

        for (size_t i = 0; i < imageFiles.size(); i += batchSize) {
            std::vector<cv::Mat>       images;
            std::vector<deploy::Image> imgBatch;
            std::vector<std::string>   imgNameBatch;

            for (size_t j = i; j < i + batchSize && j < imageFiles.size(); ++j) {
                cv::Mat image = cv::imread(imageFiles[j], cv::IMREAD_COLOR);
                cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
                images.emplace_back(image);
                imgBatch.emplace_back(image.data, image.cols, image.rows);
                imgNameBatch.emplace_back(fs::path(imageFiles[j]).filename().string());
            }

            if (images.size() != batchSize && useCudaGraph) break;

            if (i != 5) {
                cpuTimer.start();
                gpuTimer.start();
            }

            auto results = model->predict(imgBatch);

            if (i > 5) {
                gpuTimer.stop();
                cpuTimer.stop();
                count++;
            }

            if (!outputPath.empty()) {
                for (size_t j = 0; j < images.size(); ++j) {
                    cv::cvtColor(images[j], images[j], cv::COLOR_RGB2BGR);
                    visualize(images[j], results[j], labels, is_obb);
                    cv::imwrite(outputPath + "/" + imgNameBatch[j], images[j]);
                }
            }
        }

        if (count > 0) {
            std::cout << "Average infer CPU elapsed time: " << cpuTimer.milliseconds() / count << " ms" << std::endl;
            std::cout << "Average infer GPU elapsed time: " << gpuTimer.milliseconds() / count << " ms" << std::endl;
        }
    }

    std::cout << "Inference completed." << std::endl;
    return 0;
}
