#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <memory>
#include <onnxruntime_cxx_api.h>

struct Detection {
    int classId;
    std::string className;
    float confidence;
    cv::Rect box;
};

class YOLOv8Detector {
public:
    YOLOv8Detector(const std::string& modelPath, 
                   const cv::Size& inputSize = cv::Size(640, 640),
                   float confThreshold = 0.25f, 
                   float nmsThreshold = 0.45f);
    ~YOLOv8Detector();

    bool loadModel();
    std::vector<Detection> detect(const cv::Mat& frame);

private:
    std::string modelPath;
    cv::Size inputSize;
    float confThreshold;
    float nmsThreshold;
    
    // ONNX Runtime components
    std::unique_ptr<Ort::Env> env;
    std::unique_ptr<Ort::Session> session;
    
    std::vector<std::string> inputNames;
    std::vector<std::string> outputNames;
    std::vector<const char*> inputNodeNames;
    std::vector<const char*> outputNodeNames;

    std::vector<std::string> classNames;

    void preprocess(const cv::Mat& frame, std::vector<float>& inputTensorValues);
    std::vector<Detection> postprocess(const cv::Mat& frame, float* outputData, int dimensions, int rows, float scale, int pad_x, int pad_y);
    cv::Mat letterbox(const cv::Mat& src, cv::Size target_size, cv::Scalar pad_color, float& scale, int& pad_x, int& pad_y);
};
