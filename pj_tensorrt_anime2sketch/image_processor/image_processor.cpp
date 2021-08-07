/*** Include ***/
/* for general */
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <memory>

/* for OpenCV */
#include <opencv2/opencv.hpp>

/* for My modules */
#include "common_helper.h"
#include "anime_to_sketch_engine.h"
#include "image_processor.h"

/*** Macro ***/
#define TAG "ImageProcessor"
#define PRINT(...)   COMMON_HELPER_PRINT(TAG, __VA_ARGS__)
#define PRINT_E(...) COMMON_HELPER_PRINT_E(TAG, __VA_ARGS__)

/*** Global variable ***/
std::unique_ptr<Anime2SketchEngine> s_engine;

/*** Function ***/
static cv::Scalar CreateCvColor(int32_t b, int32_t g, int32_t r) {
#ifdef CV_COLOR_IS_RGB
    return cv::Scalar(r, g, b);
#else
    return cv::Scalar(b, g, r);
#endif
}

static void DrawText(cv::Mat& mat, const std::string& text, cv::Point pos, double font_scale, int32_t thickness, cv::Scalar color_front, cv::Scalar color_back, bool is_text_on_rect = true)
{
    int32_t baseline = 0;
    cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, font_scale, thickness, &baseline);
    baseline += thickness;
    pos.y += textSize.height;
    if (is_text_on_rect) {
        cv::rectangle(mat, pos + cv::Point(0, baseline), pos + cv::Point(textSize.width, -textSize.height), color_back, -1);
        cv::putText(mat, text, pos, cv::FONT_HERSHEY_SIMPLEX, font_scale, color_front, thickness);
    } else {
        cv::putText(mat, text, pos, cv::FONT_HERSHEY_SIMPLEX, font_scale, color_back, thickness * 3);
        cv::putText(mat, text, pos, cv::FONT_HERSHEY_SIMPLEX, font_scale, color_front, thickness);
    }
}

static void DrawFps(cv::Mat& mat, double time_inference, cv::Point pos, double font_scale, int32_t thickness, cv::Scalar color_front, cv::Scalar color_back, bool is_text_on_rect = true)
{
    char text[64];
    static auto time_previous = std::chrono::steady_clock::now();
    auto time_now = std::chrono::steady_clock::now();
    double fps = 1e9 / (time_now - time_previous).count();
    time_previous = time_now;
    snprintf(text, sizeof(text), "FPS: %.1f, Inference: %.1f [ms]", fps, time_inference);
    DrawText(mat, text, cv::Point(0, 0), 0.5, 2, CreateCvColor(0, 0, 0), CreateCvColor(180, 180, 180), true);
}

int32_t ImageProcessor::Initialize(const InputParam& input_param)
{
    if (s_engine) {
        PRINT_E("Already initialized\n");
        return -1;
    }

    s_engine.reset(new Anime2SketchEngine());
    if (s_engine->Initialize(input_param.work_dir, input_param.num_threads) != Anime2SketchEngine::kRetOk) {
        s_engine->Finalize();
        s_engine.reset();
        return -1;
    }
    return 0;
}

int32_t ImageProcessor::Finalize(void)
{
    if (!s_engine) {
        PRINT_E("Not initialized\n");
        return -1;
    }

    if (s_engine->Finalize() != Anime2SketchEngine::kRetOk) {
        return -1;
    }

    return 0;
}


int32_t ImageProcessor::Command(int32_t cmd)
{
    if (!s_engine) {
        PRINT_E("Not initialized\n");
        return -1;
    }

    switch (cmd) {
    case 0:
    default:
        PRINT_E("command(%d) is not supported\n", cmd);
        return -1;
    }
}


int32_t ImageProcessor::Process(cv::Mat& mat, Result& result)
{
    if (!s_engine) {
        PRINT_E("Not initialized\n");
        return -1;
    }

    Anime2SketchEngine::Result style_transfer_result;
    s_engine->Process(mat, style_transfer_result);

    DrawFps(mat, style_transfer_result.time_inference, cv::Point(0, 0), 0.5, 2, CreateCvColor(0, 0, 0), CreateCvColor(180, 180, 180), true);

    /* Return the results */
    mat = style_transfer_result.image;
    result.time_pre_process = style_transfer_result.time_pre_process;
    result.time_inference = style_transfer_result.time_inference;
    result.time_post_process = style_transfer_result.time_post_process;

    return 0;
}

