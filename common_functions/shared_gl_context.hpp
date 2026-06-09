#pragma once
#include "basic_processing.hpp"
#include "OpenGL_world.hpp"
#include "OpenCV_functions.hpp"
#include "opencv2/opencv.hpp"
#include "CV_GL_combination.hpp"
#include <GLUT/glut.h>
#include <thread>
#include <fstream>
#include <json.hpp>
#include <filesystem>
#include <string>

// 共通グローバル（シングルトン参照とフレームカウンタ）
extern MyGLApp& app_gl;
extern int frame_gl;

// 共通ビュー設定（07/08/09/11 共通）
void viewsetting();

// OBJファイル読み込みユーティリティ（07/08/09/11 共通）
void loadOBJ(GLMmodel*& model, const std::string& filepath);

// カメラデータ操作（08/09/11 共通）
void add_camID_interface(std::vector<Cam_data>& cams, std::string dir);
std::string clean_num(const std::string& s);
void add_refpoints(std::vector<Cam_data>& cams, std::string dir);
void add_refpoints_OPT_calib(std::vector<Cam_data>& cams, std::string dir);
void add_ref_image(std::vector<Cam_data>& cams, std::string dir);
void add_cam_intrinsics(std::vector<Cam_data>& cams, std::string dir);
void add_newCameraMatrix(std::vector<Cam_data>& cams);
void add_undistortedimages(std::vector<Cam_data>& cams);
void add_undistortedpoints(std::vector<Cam_data>& cams,
                           std::vector<cv::Point2d> Cam_data::* mbr_src,
                           std::vector<cv::Point2d> Cam_data::* mbr_dst);
void apply_undistortion(std::vector<Cam_data>& cams);
void add_RTvec(std::vector<Cam_data>& cams);
void add_2d_3d_point(std::vector<Cam_data>& cams,
                     std::vector<cv::Point2d> Cam_data::* mbr_pt2,
                     std::vector<cv::Point3d> Cam_data::* mbr_pt3,
                     std::string dir,
                     std::string filename);
std::vector<std::string> get_column(int column, std::string dir, std::string filename);
void add_2d_point(std::vector<Cam_data>& cams,
                  std::vector<cv::Point2d> Cam_data::* mbr_pt2,
                  std::string dir,
                  std::string filename);
void add_interface(std::vector<Cam_data>& cams,
                   std::array<double,4> Cam_data::* mbr_interface);

// erase_data はテンプレート関数のためヘッダに定義
template<typename T>
void erase_data(std::vector<Cam_data>& cams, T Cam_data::* mbr) {
    for (auto& cam : cams) {
        auto& data = cam.*mbr;
        data.erase(data.begin(), data.end());
    }
}

// 屈折・レイ計算（08/09/11 共通）
void add_Refracted_vector(std::vector<Cam_data>& cams,
                          cv::Point3d Cam_data::* mbr_from,
                          std::vector<cv::Point3d> Cam_data::* mbr_to,
                          std::vector<cv::Point3d> Cam_data::* mbr_dst,
                          double n1, double n2);
void add_Refracted_vector(std::vector<Cam_data>& cams,
                          std::vector<cv::Point3d> Cam_data::* mbr_from,
                          std::vector<cv::Point3d> Cam_data::* mbr_to,
                          std::vector<cv::Point3d> Cam_data::* mbr_dst,
                          double n1, double n2);
void add_Reversed_Refracted_vector(std::vector<Cam_data>& cams,
                                   std::vector<cv::Point3d> Cam_data::* mbr_from,
                                   std::vector<cv::Point3d> Cam_data::* mbr_to,
                                   std::vector<cv::Point3d> Cam_data::* mbr_dst,
                                   double n1, double n2);
bool multiRayLeastSquares(const std::vector<cv::Point3d>& origins,
                          const std::vector<cv::Point3d>& dirs,
                          cv::Point3d& outX,
                          double* rms = nullptr);
std::vector<cv::Point3d> estimate_all_focus_points(const std::vector<Cam_data>& cams,
                                                    const std::vector<cv::Point3d> Cam_data::* mbr_ori,
                                                    const std::vector<cv::Point3d> Cam_data::* mbr_dir,
                                                    std::vector<double>* rms_list = nullptr);
void estimate_reversed_focus_points(std::vector<Cam_data>& cams,
                                    const std::vector<cv::Point3d> Cam_data::* mbr_ori,
                                    const std::vector<cv::Point3d> Cam_data::* mbr_dir,
                                    cv::Point3d Cam_data::* mbr_dst);
void csvwriter_report(std::vector<std::vector<double>> data,
                      std::vector<double> data2,
                      std::string filename);
