#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

// カメラが利用可能か確認
std::vector<int> listAvailableCameras(int maxIndex = 10) {
    std::vector<int> availableCameras;
    for (int i = 0; i < maxIndex; ++i) {
        cv::VideoCapture cap(i);
        if (cap.isOpened()) {
            availableCameras.push_back(i);
            cap.release();
        }
    }
    return availableCameras;
}

// カメラがサポートする解像度を確認
std::vector<std::pair<int, int>> checkSupportedResolutions(int camIndex, const std::vector<std::pair<int, int>>& resolutionCandidates) {
    std::vector<std::pair<int, int>> supportedResolutions;
    cv::VideoCapture cap(camIndex);
    if (!cap.isOpened()) return supportedResolutions;

    for (const auto& res : resolutionCandidates) {
        int width = res.first;
        int height = res.second;

        cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);

        int actualWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
        int actualHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

        if (actualWidth == width && actualHeight == height) {
            supportedResolutions.emplace_back(width, height);
        }
    }
    cap.release();
    return supportedResolutions;
}

int main3() {
    // 現在の実行ディレクトリをプリント
    std::cout << "実行時ディレクトリ: " << fs::current_path() << std::endl;

    // 利用可能カメラを検出
    auto cameras = listAvailableCameras(10);
    if (cameras.empty()) {
        std::cout << "利用可能なカメラがありません\n";
        return 1;
    }

    std::cout << "利用可能なカメラ: ";
    for (int cam : cameras) std::cout << cam << " ";
    std::cout << std::endl;

    std::vector<std::pair<int, int>> resolutionCandidates = {
        {320, 240}, {640, 480}, {720, 480}, {800, 600},
        {960, 540}, {1024, 768}, {1280, 720}, {1280, 1024}, {1920, 1080}
    };

    fs::create_directory("output");

    for (int camIndex : cameras) {
        auto supported = checkSupportedResolutions(camIndex, resolutionCandidates);
        std::cout << "Camera " << camIndex << " のサポート解像度: ";
        for (const auto& res : supported) {
            std::cout << res.first << "x" << res.second << " ";
        }
        std::cout << std::endl;

        if (supported.empty()) continue;

        // 最大面積の解像度を選択
        auto best = *std::max_element(supported.begin(), supported.end(), [](auto& a, auto& b) {
            return a.first * a.second < b.first * b.second;
        });

        cv::VideoCapture cap(camIndex);
        cap.set(cv::CAP_PROP_FRAME_WIDTH, best.first);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, best.second);

        cv::Mat frame;
        cap.read(frame);

        if (!frame.empty()) {
            std::string filename = "output/camera_" + std::to_string(camIndex) + "_" +
                                   std::to_string(best.first) + "x" + std::to_string(best.second) + ".jpg";
            cv::imwrite(filename, frame);
        } else {
            std::cout << "Camera " << camIndex << " でフレーム取得失敗\n";
        }
        cap.release();
    }

    return 0;
}
