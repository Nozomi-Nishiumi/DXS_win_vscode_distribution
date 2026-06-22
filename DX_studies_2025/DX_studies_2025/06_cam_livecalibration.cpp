#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <stdexcept>

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
// カメラ識別子を取得する関数
std::string getCameraIdentifier(const std::string &device) {
    // 数値の場合は整数として扱う
    try {
        int deviceId = std::stoi(device);
        return "camera_" + std::to_string(deviceId);
    } catch (...) {
        // 数値でなければ URL とみなす
    }
    // "://" を含む場合はプロトコル部分を除去し、ホスト名部分を抽出
    size_t pos = device.find("://");
    if (pos != std::string::npos) {
        std::string remainder = device.substr(pos + 3);
        // ユーザ情報が含まれる場合は除去
        size_t at_pos = remainder.find("@");
        if (at_pos != std::string::npos) {
            remainder = remainder.substr(at_pos + 1);
        }
        // ホスト名は ':' や '/' までの部分
        size_t end_pos = remainder.find_first_of(":/");
        std::string hostname = remainder.substr(0, end_pos);
        // ドットをアンダースコアに置換
        std::replace(hostname.begin(), hostname.end(), '.', '_');
        return "camera_" + hostname;
    }
    // それ以外の場合は、英数字以外をアンダースコアに置換
    std::string safe_id;
    for (char c : device) {
        if (std::isalnum(static_cast<unsigned char>(c)))
            safe_id.push_back(c);
        else
            safe_id.push_back('_');
    }
    return "camera_" + safe_id;
}

int main6(int argc, char** argv) {
    const char* keys =
        "{help h usage ? |      | print this message }"
        "{device       | 0    | Camera device ID or URL }"
        "{width        | 1920 | Capture width }"
        "{height       | 1080  | Capture height }"
        "{grid_size    | 10,7 | Chessboard grid size (cols,rows) of inner corners }"
        "{k_filename   |      | Filename for camera matrix K }"
        "{d_filename   |      | Filename for distortion coefficients d }"
        "{interval_time| 1  | Wait interval time (ms) }"
        "{use_autoappend| false| Auto-append flag (if true, append whenever chessboard検出成功) }";

    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("Camera Calibration Tool (C++ Xcode version)");
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    // 引数取得
    std::string deviceStr = parser.get<std::string>("device");
    int capWidth = parser.get<int>("width");
    int capHeight = parser.get<int>("height");
    std::string gridSizeStr = parser.get<std::string>("grid_size");
    std::string kFilenameArg = parser.get<std::string>("k_filename");
    std::string dFilenameArg = parser.get<std::string>("d_filename");
    int intervalTime = parser.get<int>("interval_time");
    bool useAutoappend = parser.get<bool>("use_autoappend");

    // autoappend が false なら intervalTime を 10ms に設定
    if (!useAutoappend) {
        intervalTime = 10;
    }

    // grid_size のパース（例："10,7" → cols=10, rows=7）
    std::istringstream iss(gridSizeStr);
    std::string token;
    int gridCols = 0, gridRows = 0;
    if (std::getline(iss, token, ',')) {
        gridCols = std::stoi(token);
    }
    if (std::getline(iss, token, ',')) {
        gridRows = std::stoi(token);
    }
    cv::Size patternSize(gridCols, gridRows);

    // 出力ディレクトリ "output" の作成
    std::string outputDir = "output";
    std::filesystem::create_directories(outputDir);

    // カメラ識別子から出力ファイル名を設定（引数が指定されていなければ自動設定）
    std::string identifier = getCameraIdentifier(deviceStr);
    std::string kFilename = kFilenameArg.empty() ? (outputDir + "/" + identifier + "_K.csv") : kFilenameArg;
    std::string dFilename = dFilenameArg.empty() ? (outputDir + "/" + identifier + "_d.csv") : dFilenameArg;

    // カメラデバイスのオープン（整数の場合は数値に変換、それ以外は文字列としてオープン）
    cv::VideoCapture cap;
    try {
        int deviceId = std::stoi(deviceStr);
        cap.open(deviceId);
    } catch (const std::invalid_argument &e) {
        cap.open(deviceStr);
    }
    if (!cap.isOpened()) {
        std::cerr << "Cannot open camera device: " << deviceStr << std::endl;
        return -1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, capWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, capHeight);

    // チェスボード内の各交点の座標（Z=0）
    // 内部パラメータ(K)・歪み係数の推定は格子の絶対サイズに対してスケール不変なので、
    // 物理的な一辺長(square_len)は不要。単位格子(1.0刻み)で十分。
    // 実寸スケールが要る姿勢推定は 07 側で実寸4点を与えて別途行う。
    std::vector<cv::Point3f> patternPoints;
    for (int i = 0; i < patternSize.height; i++) {
        for (int j = 0; j < patternSize.width; j++) {
            patternPoints.push_back(cv::Point3f(static_cast<float>(j), static_cast<float>(i), 0));
        }
    }

    // キャリブレーション用の3D座標と画像上の2D座標を保持するコンテナ
    std::vector<std::vector<cv::Point3f>> objectPoints;
    std::vector<std::vector<cv::Point2f>> imagePoints;

    int captureCount = 0;
    cv::Size imageSize;

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Cannot capture frame." << std::endl;
            continue;
        }
        imageSize = frame.size();

        // チェスボードコーナー検出
        std::vector<cv::Point2f> corners;
        bool found = cv::findChessboardCorners(frame, patternSize, corners,
                                               cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);

        if (found) {
            std::cout << "findChessboardCorners() : True" << std::endl;
            // 精度向上のためコーナーサブピクセル化
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
                             cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.1));
            cv::drawChessboardCorners(frame, patternSize, corners, found);
        } else {
            std::cout << "findChessboardCorners() : False" << std::endl;
        }

        // 画面上にテキストを描画
        std::string text1 = "Enter: Capture (" + std::to_string(captureCount) + ")";
        std::string text2 = "ESC : Finish";
        cv::putText(frame, text1, cv::Point(10, 80), cv::FONT_HERSHEY_DUPLEX, 3,
                    cv::Scalar(0, 255, 0), 7);
        cv::putText(frame, text2, cv::Point(10, 200), cv::FONT_HERSHEY_DUPLEX, 3,
                    cv::Scalar(0, 255, 0), 7);

        resize(frame, frame, cv::Size(), 300.0/(double)frame.cols,300.0/(double)frame.cols);        cv::imshow("original", frame);

        int key = cv::waitKey(intervalTime) & 0xFF;
        // autoappend が true なら検出成功時に自動でキャプチャ、false の場合は Enter (13) キーでキャプチャ
        if ((useAutoappend && found) || (!useAutoappend && key == 13 && found)) {
            imagePoints.push_back(corners);
            objectPoints.push_back(patternPoints);
            captureCount++;
        }
        if (key == 27) { // ESC キーで終了
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();

    // 十分な画像がキャプチャされていればキャリブレーション実施
    if (!imagePoints.empty()) {
        std::cout << "calibrateCamera()" << std::endl;
        cv::Mat cam_mat, dist_coefs;
        std::vector<cv::Mat> rvecs, tvecs;
        double rms = cv::calibrateCamera(objectPoints, imagePoints, imageSize, cam_mat, dist_coefs, rvecs, tvecs);
        std::cout << "RMS = " << rms << std::endl;
        std::cout << "cam_mat = " << std::endl << cam_mat << std::endl;
        std::cout << "dist_coefs = " << dist_coefs.t() << std::endl; // 横ベクトルとして表示


        std::string savename="./output/cam_intrinsic_prameters_test.yml";
        cv::FileStorage fs(savename, cv::FileStorage::WRITE);
        fs << "cameraMatrix" << cam_mat;
        fs << "distCoeffs" << dist_coefs;
        fs << "imageSize" << imageSize;
        fs.release();

        // カメラ行列 cam_mat の CSV 出力
        std::ofstream kFile(kFilename);
        if (kFile.is_open()) {
            for (int i = 0; i < cam_mat.rows; i++) {
                for (int j = 0; j < cam_mat.cols; j++) {
                    kFile << std::fixed << std::setprecision(14) << cam_mat.at<double>(i, j);
                    if (j != cam_mat.cols - 1)
                        kFile << ",";
                }
                kFile << "\n";
            }
            kFile.close();
        } else {
            std::cerr << "Cannot open file: " << kFilename << std::endl;
        }

        // 歪み係数 dist_coefs の CSV 出力
        std::ofstream dFile(dFilename);
        if (dFile.is_open()) {
            for (int i = 0; i < dist_coefs.cols; i++) {
                dFile << std::fixed << std::setprecision(14) << dist_coefs.at<double>(0, i);
                if (i != dist_coefs.cols - 1)
                    dFile << ",";
            }
            dFile << "\n";
            dFile.close();
        } else {
            std::cerr << "Cannot open file: " << dFilename << std::endl;
        }

        // 再投影誤差の計算
        double totalError = 0;
        size_t totalPoints = 0;
        for (size_t i = 0; i < objectPoints.size(); i++) {
            std::vector<cv::Point2f> imagePoints2;
            cv::projectPoints(objectPoints[i], rvecs[i], tvecs[i], cam_mat, dist_coefs, imagePoints2);
            double err = cv::norm(imagePoints[i], imagePoints2, cv::NORM_L2);
            size_t n = objectPoints[i].size();
            totalError += err * err;
            totalPoints += n;
        }
        double meanError = std::sqrt(totalError / totalPoints);
        std::cout << "total error: " << meanError << std::endl;
    } else {
        std::cout << "findChessboardCorners() not be successful once" << std::endl;
    }

    return 0;
}
