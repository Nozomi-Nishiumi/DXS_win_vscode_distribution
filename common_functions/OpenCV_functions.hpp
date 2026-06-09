#pragma once
#include <iostream>
#include "basic_processing.hpp"
#include "opencv2/opencv.hpp"
#include <string>

using namespace std;
using namespace cv;

class Trackbar {
public:
    // 識別子（必ず setIdentifiers() で設定）
    std::string camID   = "cam0";
    std::string roleName= "ori";
    std::string name    = "cam0_ori"; // = camID + "_" + roleName（ウィンドウ名/ファイル名）

    // レンジ値
    int ch0_min = 0,   ch0_max = 180;
    int ch1_min = 0,   ch1_max = 255;
    int ch2_min = 0,   ch2_max = 255;
    int N = 1;

    // 上限値（HSVなら 180,255,255 など）
    void setMaxValues(int ch0_max_val_, int ch1_max_val_, int ch2_max_val_);
    // N の上限値（デフォルト100）
    void setNMaxValue(int n_max_val_);

    // camID / roleName を与え、name を確定
    void setIdentifiers(const std::string& camID_, const std::string& role_);

    // 任意名を直接セット（単体利用向け）
        void setName(const std::string& name_);  // 例："colortrack"
    // 設定ロード（GUIなしで使う場面用にも分離維持）
    void loadparameters();

    // トラックバー生成（値変更時に自動保存）
    // 1) Cam_data用：内部の name を使う
       void createTrackbar(int ch0_max_val_, int ch1_max_val_, int ch2_max_val_);
       // 2) 単体用：渡された名前をそのまま name として使う
       void createTrackbar(const std::string& name_,
                           int ch0_max_val_, int ch1_max_val_, int ch2_max_val_);

private:
    // 内部実装
    std::string filename() const;   // "camID_role.yml"
    void saveToFile() const;        // 変更時に即保存（外部からは呼ばない）
    static void onTrackbarChanged(int, void* userdata);
    void setupTrackbars();          // 引数なし。name をそのまま使う

    int ch0_max_val = 180, ch1_max_val = 255, ch2_max_val = 255;
    int N_max_val = 100;
};


class Cam_data {
public:
    // 1) 従来のコンストラクタ（IDを渡したら即初期化＆ロード）
       explicit Cam_data(const std::string& cam_id);

       // 2) 追加：デフォルトコンストラクタ（ID未設定・ロードもしない）
       Cam_data();

       // 3) 追加：後からIDを与えて従来と同じ初期化を実行
       void setID(const std::string& cam_id);

       // データ
       cv::Mat im_ori, im_undistorted, im_ref, im_ref_undistorted, im_track;
       cv::Mat rvec = (cv::Mat_<double>(3,1) << 0,0,0);
       cv::Mat tvec = (cv::Mat_<double>(3,1) << 0,0,0);
       cv::Mat cam_mat, new_cam_mat;
        cv::Mat dist_coefs;
        cv::Mat transmat;
       cv::Size capsize;
    cv::Point3f cam_position;
    cv::Point3d cam_position_opt;
       std::vector<cv::Point2d> refpoint2d,refpoint2d_u;
       std::vector<cv::Point3d> refpoint3d;
    std::vector<cv::Point2d> refpoint2d_opt,refpoint2d_opt_u;
    std::vector<cv::Point3d> refpoint3d_opt, refpoint3d_estimated;
    std::vector<cv::Point2d> tgtpoint2d, tgtpoint2d_u;
    std::vector<cv::Point3d> tgtpoint3d;
    std::vector<cv::Point3d> intersections,intersections2;
    std::vector<cv::Point3d> ref_on_interface, tgt_on_interface;

    std::array<double,4> interface;
    std::vector<cv::Point3d> refractedRayDirs,refractedRayDirs_tgt;
    std::vector<cv::Point3d> reversed_refractedRayDirs;
    std::vector<cv::Point3f> transport_3d{cv::Point3f(0,0,0)};
    std::vector<cv::Point3f> trackdata_3d{cv::Point3f(0,0,0)};

       // 役割ごとに用意
       Trackbar ori, oth, tgt;

       std::string camID;

   private:
       // 共通初期化（識別子設定＋ロード）を一箇所に集約
       void initFromCamID(const std::string& cam_id);
};





void colorRange(Mat src, Mat &dst, int a, int b, int c, int d, int e, int f, bool and_or=1);
void multiChannelRangeMask(const cv::Mat& src,
                           cv::Mat& dst,
                           const std::vector<cv::Point2i>& ranges,
                           bool useAnd = true);
Point2d colorTrack(Mat &src, int a, int b, int c, int d, int e, int f, bool and_or=1);
Point2d colorTrack2(Mat &src, const std::vector<cv::Point2i> &ranges, bool useAnd=true);

Point2f getContourCenter(const vector<Point>& contour);

cv::Scalar hsvScalarToBgr(const cv::Scalar& hsv_scalar);

float angleRelativeTo(const Point2f& baseVec, const Point2f& vec);

vector<Point2f> sortClockwise(const vector<Point2f>& points);

vector<Point3f> sortClockwise3D(const vector<Point3f>& points);

pair<vector<Point2f>, vector<Point3f>> matchCorrespondingPoints(
    const vector<Point2f>& imagePointsRaw,
    const vector<Point3f>& objectPointsRaw);

vector<Point2f> get_N_targetpoints(Mat& proc, int n);

bool hasDuplicatePoints(const std::vector<cv::Point2f>& points);

vector<Point3f> intersect(Point3f from,vector<Point3f> to, double surfacelevel=0.0);

void Point3d_csvwriter(vector<string> firstcolumn, vector<vector<cv::Point3d>> data,
                       std::string filename, std::string mode_label = "");
vector<vector<double>> convert_array(vector<cv::Point3d> data);

cv::Scalar averageHueRange(int hue1, int hue2);

cv::Scalar averageHSV(Scalar hsv1, Scalar hsv2);

// ======================================================================
// カメラデータ操作（08/09/11 共通）
// ======================================================================
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
// per-cam CSV 形式（pointNo, u, v, X, Y, Z）から1台分だけ読む
void add_2d_3d_point_percam(Cam_data& cam,
                             std::vector<cv::Point2d> Cam_data::* mbr_pt2,
                             std::vector<cv::Point3d> Cam_data::* mbr_pt3,
                             const std::string& csv_path);
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

// ======================================================================
// インターフェース交点・ホモグラフィ・誤差計算（08/09 共通）
// ======================================================================
void add_points_on_interface(std::vector<Cam_data>& cams,
                             const std::vector<cv::Point2d> Cam_data::* mbr,
                             std::vector<cv::Point3d> Cam_data::* mbr_dst);
void add_transformMat(std::vector<Cam_data>& cams,
                      cv::Mat Cam_data::* mbr_transformMat,
                      std::vector<cv::Point2d> Cam_data::* mbr_ref2d,
                      std::vector<cv::Point3d> Cam_data::* mbr_ref3d);
void add_points_on_interface2(std::vector<Cam_data>& cams,
                              const std::vector<cv::Point2d> Cam_data::* mbr,
                              std::vector<cv::Point3d> Cam_data::* mbr_dst,
                              const cv::Mat Cam_data::* mbr_transformMat,
                              const std::array<double,4> Cam_data::* mbr_interface);
std::vector<double> calcError(std::vector<cv::Point3d> tgt_p, std::vector<cv::Point3d> base_p);

// ======================================================================
// 屈折・レイ計算（08/09/11 共通）
// ======================================================================
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
void estimate_all_focus_points( std::vector<Cam_data>& cams,
                                                    const std::vector<cv::Point3d> Cam_data::* mbr_ori,
                                                    const std::vector<cv::Point3d> Cam_data::* mbr_dir,
                               std::vector<cv::Point3d> Cam_data::* mbr_dst,
                               std::vector<double>* rms_list = nullptr);
void estimate_reversed_focus_points(std::vector<Cam_data>& cams,
                                    const std::vector<cv::Point3d> Cam_data::* mbr_ori,
                                    const std::vector<cv::Point3d> Cam_data::* mbr_dir,
                                    cv::Point3d Cam_data::* mbr_dst);
void csvwriter_report(std::vector<std::vector<double>> data,
                      std::vector<double> data2,
                      std::string filename,
                      std::vector<cv::Point3d> cam_positions = {},
                      std::string mode_label = "");

// ======================================================================
// カメラスタンバイ（08/09/11 共通）
// ======================================================================
void standbyCamera(std::vector<Cam_data>& camera,
                   double *n,
                   std::string dir,
                   std::string dset_path);
