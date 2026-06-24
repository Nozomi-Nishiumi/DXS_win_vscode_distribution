
#include "OpenCV_functions.hpp"
#include <algorithm>
#include <iostream>

using namespace std;
using namespace cv;



void colorRange(Mat src, Mat &dst, int a, int b, int c, int d, int e, int f,bool and_or){

    Mat hue, hue1, hue2, saturation,saturation1,saturation2, value,value1,value2, hue_saturation;
    vector<Mat> singlechannels;
    split(src, singlechannels);
    threshold(singlechannels[0], hue1, a-1, 255, THRESH_BINARY);
    threshold(singlechannels[0], hue2, b, 255, THRESH_BINARY_INV);
    threshold(singlechannels[1], saturation1, c-1, 255, THRESH_BINARY);
    threshold(singlechannels[1], saturation2, d, 255, THRESH_BINARY_INV);
    threshold(singlechannels[2], value1, e-1, 255, THRESH_BINARY);
    threshold(singlechannels[2], value2, f, 255, THRESH_BINARY_INV);

    if(a<=b){
        bitwise_and(hue1, hue2, hue);
    }else{
        bitwise_or(hue1, hue2, hue);
    }

    if(c<=d){
        bitwise_and(saturation1, saturation2, saturation);
    }else{
        bitwise_or(saturation1, saturation2, saturation);
    }

    if(e<=f){
        bitwise_and(value1, value2, value);
    }else{
        bitwise_or(value1, value2, value);
    }


    if(and_or){
        bitwise_and(hue, saturation, hue_saturation);
        bitwise_and(hue_saturation, value, dst);
    }else{
        bitwise_or(hue, saturation, hue_saturation);
        bitwise_or(hue_saturation, value, dst);
    }


//    Mat dummy;
//    singlechannels[0].copyTo(dummy);
//    resize(dummy, dummy, cv::Size(), 600.0/(double)dummy.cols,600.0/(double)dummy.cols);
//    imshow("ch0",dummy);
//
//    
//    singlechannels[1].copyTo(dummy);
//    resize(dummy, dummy, cv::Size(), 600.0/(double)dummy.cols,600.0/(double)dummy.cols);
//    imshow("ch1",dummy);
//
//    singlechannels[2].copyTo(dummy);
//    resize(dummy, dummy, cv::Size(), 600.0/(double)dummy.cols,600.0/(double)dummy.cols);
//    imshow("ch2",dummy);
//
//    hue.copyTo(dummy);
//    resize(dummy, dummy, cv::Size(), 600.0/(double)dummy.cols,600.0/(double)dummy.cols);
//    imshow("ch0-bin",dummy);
//
//    saturation.copyTo(dummy);
//    resize(dummy, dummy, cv::Size(), 600.0/(double)dummy.cols,600.0/(double)dummy.cols);
//    imshow("ch1-bin",dummy);
//
//    value.copyTo(dummy);
//    resize(dummy, dummy, cv::Size(), 600.0/(double)dummy.cols,600.0/(double)dummy.cols);
//    imshow("ch2-bin",dummy);
//
//    imshow("ch0",singlechannels[0]);
//    imshow("ch1",singlechannels[1]);
//    imshow("ch2",singlechannels[2]);
//
//    imshow("ch0-bin",hue);
//    imshow("ch1-bin",saturation);
//    imshow("ch2-bin",value);

//    imshow("bin",dst);


}

void multiChannelRangeMask(const cv::Mat& src,
                           cv::Mat& dst,
                           const std::vector<cv::Point2i>& ranges,
                           bool useAnd)
{
    CV_Assert(src.channels() == static_cast<int>(ranges.size()));

    std::vector<cv::Mat> channels;
    if (src.channels() == 1) {
        channels.push_back(src);
    } else {
        cv::split(src, channels);
    }

    std::vector<cv::Mat> masks;
    for (int i = 0; i < src.channels(); ++i) {
        int minVal = ranges[i].x;
        int maxVal = ranges[i].y;

        cv::Mat lowerMask, upperMask, combined;
        cv::threshold(channels[i], lowerMask, minVal - 1, 255, cv::THRESH_BINARY);
        cv::threshold(channels[i], upperMask, maxVal, 255, cv::THRESH_BINARY_INV);

        if (minVal <= maxVal) {
            cv::bitwise_and(lowerMask, upperMask, combined);
        } else {
            cv::bitwise_or(lowerMask, upperMask, combined); // 巡回範囲
        }

        masks.push_back(combined);
    }

    dst = masks[0].clone();
    for (size_t i = 1; i < masks.size(); ++i) {
        if (useAnd)
            cv::bitwise_and(dst, masks[i], dst);
        else
            cv::bitwise_or(dst, masks[i], dst);
    }
}

Point2d colorTrack(Mat &src, int a, int b, int c, int d, int e, int f, bool and_or){
    Mat dst;
    colorRange(src, dst, a,b,c,d,e,f);

    Moments oMoments = moments(dst);
    Point2d centertmp;
    centertmp.x = oMoments.m10 / oMoments.m00;
    centertmp.y = oMoments.m01 / oMoments.m00;
    cout<<centertmp<<endl;
    return centertmp;
}

Point2d colorTrack2(Mat &src, const std::vector<cv::Point2i> &ranges, bool useAnd){
    Mat dst;
    multiChannelRangeMask(src, dst, ranges,useAnd);
    Moments oMoments = moments(dst);
    Point2d centertmp;
    centertmp.x = oMoments.m10 / oMoments.m00;
    centertmp.y = oMoments.m01 / oMoments.m00;
    cout<<centertmp<<endl;
    return centertmp;
}

Point2f getContourCenter(const vector<Point>& contour) {
    Moments m = moments(contour);
    if (m.m00 != 0){
        return Point2f(float(m.m10 / m.m00), float(m.m01 / m.m00));
    }else{
        return Point2f(0, 0);
    }
}


// HSV (Scalar) → BGR (Scalar)
cv::Scalar hsvScalarToBgr(const cv::Scalar& hsv_scalar) {
    float h = static_cast<float>(hsv_scalar[0]) * 2.0f;  // [0, 179] → [0, 360)
    float s = static_cast<float>(hsv_scalar[1]) / 255.0f;
    float v = static_cast<float>(hsv_scalar[2]) / 255.0f;

    float c = v * s;
    float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r_prime = 0, g_prime = 0, b_prime = 0;

    if (0 <= h && h < 60) {
        r_prime = c; g_prime = x; b_prime = 0;
    } else if (60 <= h && h < 120) {
        r_prime = x; g_prime = c; b_prime = 0;
    } else if (120 <= h && h < 180) {
        r_prime = 0; g_prime = c; b_prime = x;
    } else if (180 <= h && h < 240) {
        r_prime = 0; g_prime = x; b_prime = c;
    } else if (240 <= h && h < 300) {
        r_prime = x; g_prime = 0; b_prime = c;
    } else if (300 <= h && h < 360) {
        r_prime = c; g_prime = 0; b_prime = x;
    }

    uchar b = static_cast<uchar>(std::round((b_prime + m) * 255));
    uchar g = static_cast<uchar>(std::round((g_prime + m) * 255));
    uchar r = static_cast<uchar>(std::round((r_prime + m) * 255));

    return cv::Scalar(b, g, r);  // OpenCVはBGR順
}

float angleRelativeTo(const Point2f& baseVec, const Point2f& vec) {
    float angle = atan2(vec.y, vec.x) - atan2(baseVec.y, baseVec.x);
    if (angle < 0) angle += 2 * CV_PI;
    return angle;
}

vector<Point2f> sortClockwise(const vector<Point2f>& points) {
    CV_Assert(points.size() == 4);
    Point2f center(0, 0);
    for (const auto& p : points) center += p;
    center *= 1.0f / points.size();

    Point2f baseVec = points[0] - center;

    vector<Point2f> sorted = points;
    sort(sorted.begin(), sorted.end(), [&](const Point2f& a, const Point2f& b) {
        return angleRelativeTo(baseVec, a - center) < angleRelativeTo(baseVec, b - center);
    });
    return sorted;
}

vector<Point3f> sortClockwise3D(const vector<Point3f>& points) {
    CV_Assert(points.size() == 4);
    Point2f center(0, 0);
    for (const auto& p : points) center += Point2f(p.x, p.y);
    center *= 1.0f / points.size();

    Point2f baseVec = Point2f(points[0].x, points[0].y) - center;

    vector<Point3f> sorted = points;
    sort(sorted.begin(), sorted.end(), [&](const Point3f& a, const Point3f& b) {
        return angleRelativeTo(baseVec, Point2f(a.x, a.y) - center) <
               angleRelativeTo(baseVec, Point2f(b.x, b.y) - center);
    });
    return sorted;
}

pair<vector<Point2f>, vector<Point3f>> matchCorrespondingPoints(
    const vector<Point2f>& imagePointsRaw,
    const vector<Point3f>& objectPointsRaw)
{
    CV_Assert(imagePointsRaw.size() == 4 && objectPointsRaw.size() == 4);

//    vector<Point2f> dummy=imagePointsRaw;
//    for(int i=0;i<imagePointsRaw.size(); i++){
//        dummy[i].y=-imagePointsRaw[i].y;
//    }
    vector<Point2f> sorted2D = sortClockwise(imagePointsRaw);

//    dummy=sorted2D;
//    for(int i=0;i<sorted2D.size(); i++){
//        dummy[i].y=-sorted2D[i].y;
//    }
    vector<Point3f> sorted3D = sortClockwise3D(objectPointsRaw);

//    // 並び替えられた3D点のインデックスを元の順序と照合
//    vector<int> indices;
//    for (const auto& p : sorted3D) {
//        auto it = find(objectPointsRaw.begin(), objectPointsRaw.end(), p);
//        if (it == objectPointsRaw.end()) return {};
//        indices.push_back(distance(objectPointsRaw.begin(), it));
//    }
//
//    // sorted2Dをその順に並べ直す
//    vector<Point2f> reordered2D(4);
//    for (int i = 0; i < 4; ++i) {
//        reordered2D[indices[i]] = sorted2D[i];
//    }

    return { sorted2D, sorted3D };
}

vector<Point2f> get_N_targetpoints(Mat& proc, int n){
    vector<vector<Point>> contours;
    findContours(proc, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // 面積順にソート（大きい順）
    sort(contours.begin(), contours.end(),
         [](const vector<Point>& a, const vector<Point>& b) {
        return contourArea(a) > contourArea(b);
    });

    // 上位 n 個に切り捨て（コンテツが n 未満の場合は増やさない）
    if ((int)contours.size() > n) contours.resize(n);

    // 重心取得（空輪郭はスキップ → 検出0件なら空ベクタを返す）
    vector<Point2f> centers;
    for (int i = 0; i < (int)contours.size(); i++) {
        if (contours[i].empty()) continue;
        centers.push_back(getContourCenter(contours[i]));
    }
    return centers;
}

bool hasDuplicatePoints(const std::vector<cv::Point2f>& points) {
    std::set<std::pair<float, float>> seen;
    for (const auto& pt : points) {
        auto key = std::make_pair(pt.x, pt.y);
        if (seen.count(key)) return true;  // 重複が見つかった！
        seen.insert(key);
    }
    return false;  // 重複なし
}

vector<Point3f> intersect(Point3f from,vector<Point3f> to, double surfacelevel){

    vector<Point3f> intersectPoints;
    for (const auto& pt : to) {
        cv::Point3f dir = pt - from;
        if (std::abs(dir.z) < 1e-6) continue; // 水平で交差しない
        float t = (surfacelevel-from.z) / dir.z;

        cv::Point3f intersect = from + dir * t;
        intersectPoints.push_back(intersect);
    }
    return intersectPoints;
}


void Point3d_csvwriter(vector<string> firstcolumn, vector<vector<cv::Point3d>> data,
                       std::string filename, std::string mode_label){
    ofstream ofs(filename);
    if (!mode_label.empty()) {
        // mode_label は呼び出し側が "# " 以降のフルテキストを提供する
        ofs << "# " << mode_label << "\n";
    }
    if(firstcolumn.size()<1){
        firstcolumn={"frameNo."};
        for(int i=0; i<data[0].size();i++){
            firstcolumn.push_back(to_string(i));
        }
    }

    string x="_x";
    string y="_y";
    string z="_z";

    int tgt_num=(int)data.size();
    ofs <<firstcolumn[0]<<",";
    for(int i=0;i<tgt_num;i++){
        char id[20];
        sprintf(id, "%i", i+1);
        string name_x=id+x;
        string name_y=id+y;
        string name_z=id+z;
        ofs <<
        (char*)name_x.c_str()<<","<<
        (char*)name_y.c_str()<<","<<
        (char*)name_z.c_str();

        if(i!=tgt_num-1){
            ofs<<",";
        }
    }
    ofs<<endl;



    for (int i = 0; i < data[0].size(); i++){

        ofs << firstcolumn[i+1] << ",";
        for(int k=0;k<tgt_num;k++){


            ofs << data[k][i].x << ","
            <<data[k][i].y<<","
            <<data[k][i].z;
            if(k!=tgt_num-1){
                ofs<<",";
            }

        }
        ofs << endl;
    }
}

vector<vector<double>> convert_array(vector<cv::Point3d> data){
    vector<vector<double>> result;
    for(const auto i:data){
        vector<double> each={i.x,i.y,i.z};
        result.push_back(each);
    }
    return(result);
}


cv::Scalar averageHueRange(int hue1, int hue2) {
    hue1 = (hue1 + 180) % 180;
    hue2 = (hue2 + 180) % 180;

    std::vector<int> hue_values;
    int h = hue1;
    do {
        hue_values.push_back(h);
        h = (h + 1) % 180;
    } while (h != (hue2 + 1) % 180);  // 終点も含める

    int n = hue_values.size();

    // HSV配列作成（n x 1 のMat）
    cv::Mat hsv(n, 1, CV_8UC3);
    for (int i = 0; i < n; ++i) {
        hsv.at<cv::Vec3b>(i, 0) = cv::Vec3b(hue_values[i], 255, 255);
    }

    // 一括変換
    cv::Mat bgr;
    cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);

    // BGR平均を求める
    cv::Vec3d sum(0, 0, 0);
    for (int i = 0; i < n; ++i) {
        cv::Vec3b pix = bgr.at<cv::Vec3b>(i, 0);
        sum[0] += pix[0];  // B
        sum[1] += pix[1];  // G
        sum[2] += pix[2];  // R
    }

    sum /= n;

    // 平均BGR → HSV変換（1ピクセル）
    cv::Mat avg_bgr(1, 1, CV_8UC3, cv::Scalar(sum[0], sum[1], sum[2]));
    cv::Mat avg_hsv;
    cv::cvtColor(avg_bgr, avg_hsv, cv::COLOR_BGR2HSV);
    cv::Vec3b avg_hsv_val = avg_hsv.at<cv::Vec3b>(0, 0);

    return cv::Scalar(avg_hsv_val[0], avg_hsv_val[1], avg_hsv_val[2]);
}


Scalar averageHSV(Scalar hsv1, Scalar hsv2){

    Scalar blendH=averageHueRange(hsv1[0], hsv2[0]);
    int H=blendH[0];
    int S=(blendH[1]+hsv1[1]+hsv2[1])/3;
    int V=(blendH[2]+hsv1[2]+hsv2[2])/3;

    cv::Mat hsv(1, 1, CV_8UC3, cv::Scalar(H,S,V));
    cv::Mat bgr;
    cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
    Scalar outputhsv(bgr.at<cv::Vec3b>(0, 0)[0],
                     bgr.at<cv::Vec3b>(0, 0)[1],
                     bgr.at<cv::Vec3b>(0, 0)[2]);
    return outputhsv;
}


void Trackbar::setIdentifiers(const std::string& camID_, const std::string& role_) {
    camID = camID_;
    roleName = role_;
    name = camID + "_" + roleName; // ウィンドウ名 = ファイル名のベース
}

void Trackbar::setName(const std::string& name_) {
    name = name_; // 例: "colortrack"
}

void Trackbar::setMaxValues(int a, int b, int c) {
    ch0_max_val = a; ch1_max_val = b; ch2_max_val = c;
}

void Trackbar::setNMaxValue(int nmax) {
    N_max_val = nmax;
    if (N > N_max_val) N = N_max_val;
    if (N < 0) N = 0;
}

std::string Trackbar::filename() const {
    return "output/" + name + ".yml"; // 例: "output/cam0_ori.yml"（06 の出力と同じ output フォルダに統一）
}

void Trackbar::loadparameters() {
    const auto fname = filename();
      if (!std::filesystem::exists(fname)) {
          // 初回はデフォルト値でファイル新規作成
          saveToFile();
          return;
      }

    cv::FileStorage fs(filename(), cv::FileStorage::READ);
    if (!fs.isOpened()) {
        // ファイルが存在しない場合、新規作成して保存
        saveToFile();
        return;
    }
    fs["ch0_min"] >> ch0_min; fs["ch0_max"] >> ch0_max;
    fs["ch1_min"] >> ch1_min; fs["ch1_max"] >> ch1_max;
    fs["ch2_min"] >> ch2_min; fs["ch2_max"] >> ch2_max;
    fs["N"] >> N;
}

void Trackbar::saveToFile() const {
    std::filesystem::create_directories("output"); // 出力先フォルダが無ければ作成
    cv::FileStorage fs(filename(), cv::FileStorage::WRITE);
    fs << "ch0_min" << ch0_min << "ch0_max" << ch0_max
       << "ch1_min" << ch1_min << "ch1_max" << ch1_max
       << "ch2_min" << ch2_min << "ch2_max" << ch2_max
       << "N"      << N;
}

void Trackbar::onTrackbarChanged(int, void* userdata) {
    auto* self = static_cast<Trackbar*>(userdata);
    self->saveToFile(); // 変更のたびに即保存
}

void Trackbar::setupTrackbars() {
    cv::namedWindow(name, cv::WINDOW_NORMAL);
    cv::resizeWindow(name, 600, 350);

    cv::createTrackbar("ch0 min", name, &ch0_min, ch0_max_val, onTrackbarChanged, this);
    cv::createTrackbar("ch0 max", name, &ch0_max, ch0_max_val, onTrackbarChanged, this);
    cv::createTrackbar("ch1 min", name, &ch1_min, ch1_max_val, onTrackbarChanged, this);
    cv::createTrackbar("ch1 max", name, &ch1_max, ch1_max_val, onTrackbarChanged, this);
    cv::createTrackbar("ch2 min", name, &ch2_min, ch2_max_val, onTrackbarChanged, this);
    cv::createTrackbar("ch2 max", name, &ch2_max, ch2_max_val, onTrackbarChanged, this);

    // 追加：N のトラックバー
    cv::createTrackbar("N", name, &N, N_max_val, onTrackbarChanged, this);
}

void Trackbar::createTrackbar(int a, int b, int c) {
    setMaxValues(a, b, c);
    setupTrackbars(); // 既に setIdentifiers() か setName() で name が決まっている前提
}

void Trackbar::createTrackbar(const std::string& name_,
                              int a, int b, int c) {
    setName(name_);
    setMaxValues(a, b, c);
    setupTrackbars();
}

void Cam_data::initFromCamID(const std::string& cam_id) {
    camID = cam_id;

    // 各 Trackbar に camID と role を設定
    ori.setIdentifiers(camID, "ori");
    oth.setIdentifiers(camID, "oth");
    tgt.setIdentifiers(camID, "tgt");

    // 設定をロード（ファイルが無ければ Trackbar 側で新規作成される前提の実装）
    ori.loadparameters();
    oth.loadparameters();
    tgt.loadparameters();
}

// 従来のコンストラクタ：IDを与えたら即初期化＆ロード
Cam_data::Cam_data(const std::string& cam_id) {
    initFromCamID(cam_id);
}

// 追加：デフォルトコンストラクタ（ロードしない）
Cam_data::Cam_data() = default;

// 追加：後からIDを与える
void Cam_data::setID(const std::string& cam_id) {
    initFromCamID(cam_id);
}

// ======================================================================
// カメラデータ操作（08/09/11 共通）
// ======================================================================

void add_camID_interface(std::vector<Cam_data>& cams, std::string dir){

    std::vector<std::string> setting_path = basic_processing::scan_filepath(dir, {"camera_setting"}, "");

    std::vector<std::vector<std::string>> datasheet = basic_processing::csvloader_str(setting_path[0]);

    int N = (int)datasheet[0].size()-1;
    cams.resize(N);

    for (int i = 0; i < N; i++) {
        cams[i].setID(datasheet[1][i+1]);
        cams[i].interface[0]=stod(datasheet[2][i+1]);
        cams[i].interface[1]=stod(datasheet[3][i+1]);
        cams[i].interface[2]=stod(datasheet[4][i+1]);
        cams[i].interface[3]=stod(datasheet[5][i+1]);
    }
}

std::string clean_num(const std::string& s) {
    auto is_valid_char = [](unsigned char c) {
        return std::isdigit(c) || c == '.' || c == '+' || c == '-' || c == 'e' || c == 'E';
    };

    std::string out;
    for (char c : s) {
        if (is_valid_char(c)) out += c;
    }
    return out;
}

void add_refpoints(std::vector<Cam_data>& cams, std::string dir){

    std::vector<std::string> ref_path = basic_processing::scan_filepath(dir, {"reference_points_near."}, "");

    std::vector<std::vector<std::string>> ref = basic_processing::csvloader_str(ref_path[0]);

    int N = (int)cams.size();
    for (int i = 1; i < (int)ref.size(); i++) {
        const std::vector<std::string>& row = ref[i];

        if (row.empty()) continue;
        if ((int)row.size() < 3 + 2 * N) continue;

        try {
            for (int j = 0; j < N; j++) {
                cv::Point2d pt2d(
                                 std::stod(clean_num(row[1 + 2*j])),
                                 std::stod(clean_num(row[2 + 2*j]))
                                 );

                cv::Point3d pt3d(
                                 std::stod(clean_num(row[1 + 2*N])),
                                 std::stod(clean_num(row[2 + 2*N])),
                                 std::stod(clean_num(row[3 + 2*N]))
                                 );

                cams[j].refpoint2d.push_back(pt2d);
                cams[j].refpoint3d.push_back(pt3d);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Warning: CSV row " << i
            << " is invalid (" << e.what() << "), skipped.\n";
            continue;
        }
    }
}

void add_refpoints_OPT_calib(std::vector<Cam_data>& cams, std::string dir){

    std::vector<std::string> ref_path = basic_processing::scan_filepath(dir, {"reference_points_far."}, "");

    std::vector<std::vector<std::string>> ref = basic_processing::csvloader_str(ref_path[0]);

    int N = (int)cams.size();
    for (int i = 1; i < (int)ref.size(); i++) {
        const std::vector<std::string>& row = ref[i];

        if (row.empty()) continue;
        if ((int)row.size() < 3 + 2 * N) continue;

        try {
            for (int j = 0; j < N; j++) {
                cv::Point2d pt2d(
                                 std::stod(clean_num(row[1 + 2*j])),
                                 std::stod(clean_num(row[2 + 2*j]))
                                 );

                cv::Point3d pt3d(
                                 std::stod(clean_num(row[1 + 2*N])),
                                 std::stod(clean_num(row[2 + 2*N])),
                                 std::stod(clean_num(row[3 + 2*N]))
                                 );

                cams[j].refpoint2d_opt.push_back(pt2d);
                cams[j].refpoint3d_opt.push_back(pt3d);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Warning: CSV row " << i
            << " is invalid (" << e.what() << "), skipped.\n";
            continue;
        }
    }
}

void add_ref_image(std::vector<Cam_data>& cams, std::string dir){

    // 対応拡張子（小文字・大文字両方を試す）。OpenCV imread が
    // 内容から自動判別するので、拡張子は単にファイル探索用。
    static const std::vector<std::string> kImageExts = {
        ".jpg", ".jpeg", ".png", ".tiff", ".tif", ".bmp",
        ".JPG", ".JPEG", ".PNG", ".TIFF", ".TIF", ".BMP"
    };

    for (int i = 0; i < (int)cams.size(); i++){
        std::string found_path;
        for (const auto& ext : kImageExts) {
            std::string imagename = std::to_string(i) + ext;
            std::vector<std::string> im_path = basic_processing::scan_filepath(
                dir + "/reference_data", {imagename}, "");
            if (!im_path.empty()) { found_path = im_path[0]; break; }
        }
        if (found_path.empty()) {
            std::cerr << "[add_ref_image] cam " << i
                      << ": no image found in " << dir
                      << "/reference_data (tried jpg/jpeg/png/tiff/tif/bmp)\n";
            continue;
        }
        cams[i].im_ref = imread(found_path);
        if (cams[i].im_ref.empty()) {
            std::cerr << "[add_ref_image] cam " << i
                      << ": imread failed for " << found_path << "\n";
            continue;
        }

        Mat im=cams[i].im_ref;
        // NEAR refpoint (refpoint2d) を描画
        for(int j=0;j<(int)cams[i].refpoint2d.size();j++){
            Point2d p2=cams[i].refpoint2d[j];
            Point3d p3=cams[i].refpoint3d[j];
            circle(im, p2, 5, Scalar(255,255,255), -1);
            putText(im,to_string(j),p2,
                    FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),3);

            std::ostringstream x;
            x << std::fixed << std::setprecision(1) << p3.x;
            std::ostringstream y;
            y << std::fixed << std::setprecision(1) << p3.y;
            std::ostringstream z;
            z << std::fixed << std::setprecision(1) << p3.z;

            string point3d=x.str()+", "+y.str()+", "+z.str();
            putText(im,point3d,Point2d(p2.x,p2.y+30),
                    FONT_HERSHEY_SIMPLEX,1,Scalar(100,255,100),3);
        }
        // FAR refpoint (refpoint2d_opt) を NEAR と同様の様式で描画。
        // 歪み対応: raw im_ref に描画 → 後段の add_undistortedimages で画像と
        // 一緒に歪み補正されるので、表示される im_ref_undistorted 上では
        // 自動的に refpoint2d_opt_u 相当の位置に来る。
        for(int j=0;j<(int)cams[i].refpoint2d_opt.size();j++){
            Point2d p2=cams[i].refpoint2d_opt[j];
            Point3d p3=cams[i].refpoint3d_opt[j];
            circle(im, p2, 5, Scalar(255,255,255), -1);
            putText(im,to_string(j),p2,
                    FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),3);

            std::ostringstream x;
            x << std::fixed << std::setprecision(1) << p3.x;
            std::ostringstream y;
            y << std::fixed << std::setprecision(1) << p3.y;
            std::ostringstream z;
            z << std::fixed << std::setprecision(1) << p3.z;

            string point3d=x.str()+", "+y.str()+", "+z.str();
            putText(im,point3d,Point2d(p2.x,p2.y+30),
                    FONT_HERSHEY_SIMPLEX,1,Scalar(100,255,100),3);
        }

    }
}

void add_cam_intrinsics(std::vector<Cam_data>& cams, std::string dir){
    std::string filename = "/cam_intrinsic_prameters_new.yml";
    for (int i = 0; i < (int)cams.size(); i++){
        std::string yml_dir = basic_processing::scan_filepath(
                                                              dir + "/cam_info", { "setting" + cams[i].camID }, "", 1, 0
                                                              )[0];

        std::string yml = yml_dir + filename;
        cv::FileStorage fs(yml, cv::FileStorage::READ);
        fs["cameraMatrix"] >> cams[i].cam_mat;
        fs["distCoeffs"]   >> cams[i].dist_coefs;
        fs.release();
    }
}

void add_newCameraMatrix(std::vector<Cam_data>& cams){
    for (auto& cam : cams) {

        cam.new_cam_mat = cv::getOptimalNewCameraMatrix(cam.cam_mat,
                                                        cam.dist_coefs,
                                                        cam.im_ref.size(),
                                                        0,
                                                        cam.im_ref.size(),
                                                        0);
    }
}

void add_undistortedimages(std::vector<Cam_data>& cams){
    for (auto& cam : cams) {

        cv::undistort(cam.im_ref,
                      cam.im_ref_undistorted,
                      cam.cam_mat,
                      cam.dist_coefs,
                      cam.new_cam_mat);
    }
}

void add_undistortedpoints(std::vector<Cam_data>& cams,
                           std::vector<cv::Point2d> Cam_data::* mbr_src,
                           std::vector<cv::Point2d> Cam_data::* mbr_dst){
    for (auto& cam : cams) {
        auto& pointsrc = cam.*mbr_src;

        if(pointsrc.size()>0){
            cv::undistortPoints(pointsrc, cam.*mbr_dst, cam.cam_mat, cam.dist_coefs, cv::noArray(), cam.new_cam_mat);
        }else{
            cam.*mbr_dst=pointsrc;
        }
    }
}

void apply_undistortion(std::vector<Cam_data>& cams){
    for (auto& cam : cams) {

        cam.new_cam_mat = cv::getOptimalNewCameraMatrix(
                                                                cam.cam_mat, cam.dist_coefs, cam.im_ref.size(), 0, cam.im_ref.size(), 0
                                                                );
        cv::undistort(cam.im_ref,
                      cam.im_ref_undistorted,
                      cam.cam_mat,
                      cam.dist_coefs,
                      cam.new_cam_mat);

        cv::undistortPoints(cam.refpoint2d, cam.refpoint2d_u, cam.cam_mat, cam.dist_coefs, cv::noArray(), cam.new_cam_mat);

        cv::undistortPoints(cam.refpoint2d_opt, cam.refpoint2d_opt_u, cam.cam_mat, cam.dist_coefs, cv::noArray(), cam.new_cam_mat);

        for(int j=0;j<(int)cam.refpoint2d_opt_u.size();j++){
            circle(cam.im_ref_undistorted, cam.refpoint2d_opt_u[j], 5, Scalar(255,255,255), -1);
            circle(cam.im_ref_undistorted, cam.refpoint2d_opt[j], 5, Scalar(15,255,25), -1);
            putText(cam.im_ref_undistorted,to_string(j),cam.refpoint2d_opt[j],
                    FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),3);
        }

        for(int j=0;j<(int)cam.refpoint2d_opt.size();j++){
            circle(cam.im_ref, cam.refpoint2d_opt_u[j], 5, Scalar(255,255,255), -1);
            circle(cam.im_ref, cam.refpoint2d_opt[j], 5, Scalar(15,255,25), -1);
            putText(cam.im_ref_undistorted,to_string(j),cam.refpoint2d_opt[j],
                    FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),3);
        }

    }
}

void add_RTvec(std::vector<Cam_data>& cams){
    for (auto& cam : cams) {

        bool success = cv::solvePnP(cam.refpoint3d,
                                    cam.refpoint2d,
                                    cam.cam_mat,
                                    cam.dist_coefs,
                                    cam.rvec,
                                    cam.tvec,
                                    false,
                                    cv::SOLVEPNP_ITERATIVE);

        if (!success) {
            std::cerr << "solvePnP failed!" << std::endl;
            std::exit(0);
        }

        cv::Mat R;
        cv::Rodrigues(cam.rvec, R);
        cv::Mat cameraPosition = -R.t() * cam.tvec;
        cam.cam_position = cv::Point3d(cameraPosition);
    }
}

void add_2d_3d_point(std::vector<Cam_data>& cams,
                     std::vector<cv::Point2d> Cam_data::* mbr_pt2,
                     std::vector<cv::Point3d> Cam_data::* mbr_pt3,
                     std::string dir,
                     std::string filename){

    std::vector<std::string> ref_path = basic_processing::scan_filepath(dir, {filename}, "");

    std::vector<std::vector<std::string>> ref = basic_processing::csvloader_str(ref_path[0]);

    int N = (int)cams.size();
    for (int i = 1; i < (int)ref.size(); i++) {
        const std::vector<std::string>& row = ref[i];

        if (row.empty()) continue;
        if ((int)row.size() < 3 + 2 * N) continue;

        try {
            for (int j = 0; j < N; j++) {
                auto& pt2 = cams[j].*mbr_pt2;
                auto& pt3 = cams[j].*mbr_pt3;

                cv::Point2d pt2d(
                                 std::stod(row[1 + 2*j]),
                                 std::stod(row[2 + 2*j])
                                 );

                cv::Point3d pt3d(
                                 std::stod(row[1 + 2*N]),
                                 std::stod(row[2 + 2*N]),
                                 std::stod(row[3 + 2*N])
                                 );

                pt2.push_back(pt2d);
                pt3.push_back(pt3d);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Warning: CSV row " << i
            << " is invalid (" << e.what() << "), skipped.\n";
            continue;
        }
    }
}

// per-cam CSV 形式（pointNo, u, v, X, Y, Z）から1台分だけ読み込む
void add_2d_3d_point_percam(Cam_data& cam,
                             std::vector<cv::Point2d> Cam_data::* mbr_pt2,
                             std::vector<cv::Point3d> Cam_data::* mbr_pt3,
                             const std::string& csv_path) {
    std::ifstream f(csv_path);
    if (!f.is_open()) {
        std::cerr << "add_2d_3d_point_percam: cannot open " << csv_path << "\n";
        return;
    }
    std::string line;
    std::getline(f, line); // ヘッダ行をスキップ
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> row;
        while (std::getline(ss, token, ',')) {
            token.erase(std::remove_if(token.begin(), token.end(),
                        [](unsigned char c){ return std::isspace(c) || c == '\r'; }),
                        token.end());
            row.push_back(token);
        }
        if ((int)row.size() < 6) continue;
        try {
            cv::Point2d pt2d(std::stod(row[1]), std::stod(row[2]));
            cv::Point3d pt3d(std::stod(row[3]), std::stod(row[4]), std::stod(row[5]));
            (cam.*mbr_pt2).push_back(pt2d);
            (cam.*mbr_pt3).push_back(pt3d);
        } catch (const std::exception& e) {
            std::cerr << "add_2d_3d_point_percam: parse error (" << e.what() << "), skipped\n";
        }
    }
}

std::vector<std::string> get_column(int column, std::string dir, std::string filename){

    std::vector<std::string> ref_path = basic_processing::scan_filepath(dir, {filename}, "");
    std::vector<std::vector<std::string>> ref = basic_processing::csvloader_str(ref_path[0]);

    vector<string> result;
    for (const auto& row:ref) {

        if (std::any_of(row.begin(), row.end(),
                          [](const std::string& cell) { return cell.empty(); })) {
              continue;
          }
        
        try {
            result.push_back(row[column]);
        } catch (const std::exception& e) {
            std::cerr << "Warning: CSV row  is invalid (" << e.what() << "), skipped.\n";
            continue;
        }

    }
    return result;
}

void add_2d_point(std::vector<Cam_data>& cams,
                  std::vector<cv::Point2d> Cam_data::* mbr_pt2,
                  std::string dir,
                  std::string filename){

    std::vector<std::string> ref_path = basic_processing::scan_filepath(dir, {filename}, "");

    std::vector<std::vector<std::string>> ref = basic_processing::csvloader_str(ref_path[0]);

    int N = (int)cams.size();
    for (int i = 1; i < (int)ref.size(); i++) {
        const std::vector<std::string>& row = ref[i];

        if (row.empty()) continue;

        try {
            for (int j = 0; j < N; j++) {
                auto& pt2 = cams[j].*mbr_pt2;

                cv::Point2d pt2d(
                                 std::stod(row[1 + 2*j]),
                                 std::stod(row[2 + 2*j])
                                 );

                pt2.push_back(pt2d);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Warning: CSV row " << i
            << " is invalid (" << e.what() << "), skipped.\n";
            continue;
        }
    }
}

void add_interface(std::vector<Cam_data>& cams,
                   std::array<double,4> Cam_data::* mbr_interface){
    for (auto& cam : cams){
        auto& interface = cam.*mbr_interface;

        interface[0] = 0.0; // a
        interface[1] = 1.0; // b
        interface[2] = 0.0; // c
        interface[3] = 0.0; // d
    }
}

// ======================================================================
// 屈折・レイ計算（08/09/11 共通） -- 内部ヘルパー
// ======================================================================

static inline cv::Point3d normalize(const cv::Point3d& v, double eps=1e-15){
    double n2 = v.dot(v);
    if (n2 < eps) return cv::Point3d(0,0,0);
    return v * (1.0 / std::sqrt(n2));
}

static inline double sin_between(const cv::Point3d& a, const cv::Point3d& b){
    double na = std::sqrt(a.dot(a)), nb = std::sqrt(b.dot(b));
    if(na < 1e-12 || nb < 1e-12) return 0.0;
    cv::Point3d c = a.cross(b);
    return std::sqrt(c.dot(c)) / (na*nb);
}

static inline cv::Point3d rotate_in_plane(const cv::Point3d& a, const cv::Point3d& b, double rad){
    cv::Point3d axis = a.cross(b);
    double n = std::sqrt(axis.dot(axis));
    if(n < 1e-12) return a;
    cv::Point3d k = axis * (1.0/n);
    double c = std::cos(rad), s = std::sin(rad);
    cv::Point3d kxa = k.cross(a);
    double kda = k.dot(a);
    return a*c + kxa*s + k*(kda*(1.0-c));
}

static inline std::vector<cv::Point3d> operator-(const cv::Point3d& v, const std::vector<cv::Point3d>& vs){
    std::vector<cv::Point3d> out; out.reserve(vs.size());
    for(const auto& e : vs) out.emplace_back(v - e);
    return out;
}
static inline std::vector<cv::Point3d> operator-(const std::vector<cv::Point3d>& a, const std::vector<cv::Point3d>& b){
    std::vector<cv::Point3d> out; out.reserve(a.size());
    for(size_t i=0; i<a.size(); ++i) out.emplace_back(a[i] - b[i]);
    return out;
}

// ======================================================================
// 屈折・レイ計算（08/09/11 共通） -- 本体
// ======================================================================

void add_Refracted_vector( std::vector<Cam_data>& cams,
                          cv::Point3d Cam_data::* mbr_from,
                          std::vector<cv::Point3d> Cam_data::* mbr_to,
                          std::vector<cv::Point3d> Cam_data::* mbr_dst,
                          double n1,
                          double n2
){
    for(auto& cam:cams){
         cv::Point3d normal={cam.interface[0], cam.interface[1], cam.interface[2]};
        auto& from = cam.*mbr_from;
        auto& to   = cam.*mbr_to;

        vector<cv::Point3d> vec_n1 = from - to;
        vector<cv::Point3d> vec_n2(vec_n1.size());

        for(int i=0; i<(int)vec_n1.size(); i++){
            cv::Point3d v1 = vec_n1[i];
            if(normal.dot(v1)>0){ normal = normal*(-1); }
            const double sin_n1 = sin_between(normal, v1);
            const double sin_n2 = sin_n1*n1/n2;
            if(sin_n1 > 1.0){
                cerr << "Total internal reflection occurred\n";
                vec_n1[i] = cv::Point3d(0,0,0);
                continue;
            }
            const double rad_n2 = asin(sin_n2);
            cv::Point3d v2 = rotate_in_plane(normal, -v1, rad_n2);
            const double sin_n2_check = sin_between(-normal, v2);
            vec_n2[i] = v2;
            cout << "normal:"<<normal << endl;
        }
        cam.*mbr_dst = vec_n2;
    }
}

void add_Refracted_vector( std::vector<Cam_data>& cams,
                          std::vector<cv::Point3d> Cam_data::* mbr_from,
                          std::vector<cv::Point3d> Cam_data::* mbr_to,
                          std::vector<cv::Point3d> Cam_data::* mbr_dst,
                          double n1,
                          double n2
){
    for(auto& cam:cams){
        cv::Point3d normal={cam.interface[0], cam.interface[1], cam.interface[2]};
        auto& from = cam.*mbr_from;
        auto& to   = cam.*mbr_to;

        vector<cv::Point3d> vec_n1 = from - to;
        vector<cv::Point3d> vec_n2(vec_n1.size());

        for(int i=0; i<(int)vec_n1.size(); i++){
            cv::Point3d v1 = vec_n1[i];
            if(normal.dot(v1)>0){ normal = normal*(-1); }

            const double sin_n1 = sin_between(normal, v1);
            const double sin_n2 = sin_n1*n1/n2;
            if(sin_n1 > 1.0){
                cerr << "Total internal reflection occurred\n";
                vec_n1[i] = cv::Point3d(0,0,0);
                continue;
            }
            const double rad_n2 = asin(sin_n2);
            cv::Point3d v2 = rotate_in_plane(normal, -v1, rad_n2);
            const double sin_n2_check = sin_between(-normal, v2);
            vec_n2[i] = v2;
            cout << "normal:"<<normal << endl;
        }
        cam.*mbr_dst = vec_n2;
    }
}

void add_Reversed_Refracted_vector( std::vector<Cam_data>& cams,
                                   std::vector<cv::Point3d> Cam_data::* mbr_from,
                                   std::vector<cv::Point3d> Cam_data::* mbr_to,
                                   std::vector<cv::Point3d> Cam_data::* mbr_dst,
                                   double n1,
                                   double n2
){
    for(auto& cam : cams){
        const cv::Point3d normal = {cam.interface[0], cam.interface[1], cam.interface[2]};
        auto& from = cam.*mbr_from;
        auto& to   = cam.*mbr_to;

        vector<cv::Point3d> vec_n2 = from - to;
        vector<cv::Point3d> vec_n1(vec_n2.size());

        for(int i=0; i<(int)vec_n2.size(); i++){
            cv::Point3d v2 = vec_n2[i];
            const double sin_n2 = sin_between(-normal, v2);
            const double sin_n1 = sin_n2 * n2 / n1;
            if(sin_n1 > 1.0){
                cerr << "Total internal reflection occurred\n";
                vec_n1[i] = cv::Point3d(0,0,0);
                continue;
            }
            const double rad_n1 = asin(sin_n1);
            cv::Point3d v1 = rotate_in_plane(normal, -v2, rad_n1);
            vec_n1[i] = v1;
            const double sin_check = sin_between(normal, v1);
            cout << "sin_n1_check=" << sin_check << endl;
        }
        cam.*mbr_dst = vec_n1;
    }
}

bool multiRayLeastSquares(
    const std::vector<cv::Point3d>& origins,
    const std::vector<cv::Point3d>& dirs,
    cv::Point3d& outX,
    double* rms
){
    const size_t n = std::min(origins.size(), dirs.size());
    if (n < 2) return false;

    cv::Matx33d A = cv::Matx33d::zeros();
    cv::Vec3d b(0,0,0);
    const cv::Matx33d I = cv::Matx33d::eye();

    for (size_t i=0; i<n; ++i){
        cv::Point3d d_unit = normalize(dirs[i]);
        if (d_unit.x==0.0 && d_unit.y==0.0 && d_unit.z==0.0) continue;
        cv::Vec3d d(d_unit.x, d_unit.y, d_unit.z);
        cv::Matx33d ddT(d[0]*d[0], d[0]*d[1], d[0]*d[2],
                        d[1]*d[0], d[1]*d[1], d[1]*d[2],
                        d[2]*d[0], d[2]*d[1], d[2]*d[2]);
        cv::Matx33d Pi = I - ddT;
        cv::Vec3d p(origins[i].x, origins[i].y, origins[i].z);
        A += Pi;
        b += Pi * p;
    }

    cv::Mat A_mat(3,3,CV_64F,(void*)A.val);
    cv::Mat b_mat(3,1,CV_64F);
    b_mat.at<double>(0)=b[0]; b_mat.at<double>(1)=b[1]; b_mat.at<double>(2)=b[2];
    cv::Mat x_mat;
    if (!cv::solve(A_mat, b_mat, x_mat, cv::DECOMP_SVD)) return false;

    cv::Vec3d x(x_mat.at<double>(0), x_mat.at<double>(1), x_mat.at<double>(2));
    outX = cv::Point3d(x[0], x[1], x[2]);

    if (rms){
        double sum2=0; size_t m=0;
        const cv::Matx33d I3 = cv::Matx33d::eye();
        for (size_t i=0; i<n; ++i){
            cv::Point3d d_unit = normalize(dirs[i]);
            if (d_unit.x==0.0 && d_unit.y==0.0 && d_unit.z==0.0) continue;
            cv::Vec3d d(d_unit.x, d_unit.y, d_unit.z);
            cv::Matx33d ddT(d[0]*d[0], d[0]*d[1], d[0]*d[2],
                            d[1]*d[0], d[1]*d[1], d[1]*d[2],
                            d[2]*d[0], d[2]*d[1], d[2]*d[2]);
            cv::Matx33d Pi = I3 - ddT;
            cv::Vec3d r = Pi * (x - cv::Vec3d(origins[i].x, origins[i].y, origins[i].z));
            sum2 += r.dot(r);
            ++m;
        }
        *rms = (m>0)? std::sqrt(sum2/m) : std::numeric_limits<double>::quiet_NaN();
    }
    return true;
}

void estimate_all_focus_points(
     std::vector<Cam_data>& cams,
    const std::vector<cv::Point3d> Cam_data::* mbr_ori,
    const std::vector<cv::Point3d> Cam_data::* mbr_dir,
          std::vector<cv::Point3d> Cam_data::* mbr_dst,
          std::vector<double>* rms_list
){
    std::vector<cv::Point3d> results;
    if (cams.empty()) return ;
    const size_t X = std::min((cams[0].*mbr_ori).size(), (cams[0].*mbr_dir).size());
    results.resize(X);
    if (rms_list) rms_list->resize(X);

    for (size_t j=0; j<X; ++j){
        std::vector<cv::Point3d> origins, dirs;
        origins.reserve(cams.size());
        dirs.reserve(cams.size());
        for (const auto& cam : cams){
            const auto& ori = cam.*mbr_ori;
            const auto& dir = cam.*mbr_dir;
            if (j < ori.size() && j < dir.size()){
                origins.push_back(ori[j]);
                dirs.push_back(dir[j]);
            }
        }
        cv::Point3d Xj; double rms;
        bool ok = multiRayLeastSquares(origins, dirs, Xj, &rms);
        results[j] = ok ? Xj : cv::Point3d(0,0,0);
        if (rms_list) (*rms_list)[j] = ok ? rms : std::numeric_limits<double>::quiet_NaN();
    }

    for(auto& cam:cams){
        cam.*mbr_dst =results;
    }
}

void estimate_reversed_focus_points(std::vector<Cam_data>& cams,
                                    const std::vector<cv::Point3d> Cam_data::* mbr_ori,
                                    const std::vector<cv::Point3d> Cam_data::* mbr_dir,
                                    cv::Point3d Cam_data::* mbr_dst){
    for (auto& cam : cams){
        cv::Point3d Xj; double rms;
        bool ok = multiRayLeastSquares(cam.*mbr_ori, cam.*mbr_dir, Xj, &rms);
        cam.*mbr_dst = ok ? Xj : cv::Point3d(0,0,0);
    }
}

// ======================================================================
// インターフェース交点・ホモグラフィ・誤差計算（08/09 共通）
// ======================================================================

static std::vector<cv::Point3d> tgtvector(Cam_data& cam,
                                          const std::vector<cv::Point2d> Cam_data::* mbr)
{
    const auto& pts2d = cam.*mbr;
    std::vector<cv::Point2d> normPoints;
    cv::undistortPoints(pts2d, normPoints, cam.cam_mat, cam.dist_coefs);

    std::vector<cv::Point3d> camPoints;
    camPoints.reserve(normPoints.size());
    for (const auto& p : normPoints)
        camPoints.emplace_back(p.x, p.y, 1.0);

    cv::Mat R;
    cv::Rodrigues(cam.rvec, R);
    std::vector<cv::Point3d> exportdata;
    exportdata.reserve(camPoints.size());
    for (const auto& pt : camPoints) {
        cv::Mat X_cam = (cv::Mat_<double>(3,1) << pt.x, pt.y, pt.z);
        cv::Mat worldPt = R.t() * (X_cam - cam.tvec);
        exportdata.push_back(cv::Point3d(worldPt));
    }
    return exportdata;
}

static int intersectLinePlane(
    const cv::Point3d& from,
    const cv::Point3d& to,
    std::array<double,4> p,
    cv::Point3d& X,
    double* t_out = nullptr,
    double eps = 1e-12)
{
    const double a = p[0], b = p[1], c = p[2], d = p[3];
    const cv::Point3d dir = to - from;
    const double denom = a*dir.x + b*dir.y + c*dir.z;
    const double num   = d - (a*from.x + b*from.y + c*from.z);
    if (std::fabs(denom) < eps) {
        if (std::fabs(num) < eps) { X = from; if (t_out) *t_out = 0.0; return 2; }
        return 0;
    }
    const double t = num / denom;
    X = cv::Point3d(from.x + t*dir.x, from.y + t*dir.y, from.z + t*dir.z);
    if (t_out) *t_out = t;
    return 1;
}

void add_points_on_interface(
    std::vector<Cam_data>& cams,
    const std::vector<cv::Point2d> Cam_data::* mbr,
    std::vector<cv::Point3d> Cam_data::* mbr_dst)
{
    for (auto& cam : cams) {
        const cv::Point3d from = cam.cam_position;
        std::vector<cv::Point3d> to = tgtvector(cam, mbr);
        auto& interface_pts3d = cam.*mbr_dst;
        interface_pts3d.clear();
        interface_pts3d.reserve(to.size());
        for (const auto& tip : to) {
            cv::Point3d X; double t;
            if (intersectLinePlane(from, tip, cam.interface, X, &t) == 1 && t >= 0.0)
                interface_pts3d.push_back(X);
        }
    }
}

void add_transformMat(
    std::vector<Cam_data>& cams,
    cv::Mat Cam_data::* mbr_transformMat,
    std::vector<cv::Point2d> Cam_data::* mbr_ref2d,
    std::vector<cv::Point3d> Cam_data::* mbr_ref3d)
{
    for (auto& cam : cams) {
        std::vector<int> plane_axes, vertical_axis;
        for (int i = 0; i < 3; i++) {
            if (cam.interface[i] < 0.000001) plane_axes.push_back(i);
            else vertical_axis.push_back(i);
        }
        if (plane_axes.size() != 2) { cout << "axes search failed..." << endl; exit(0); }
        std::vector<cv::Point2d>& src = cam.*mbr_ref2d;
        std::vector<cv::Point2d> dist;
        for (auto r3d : cam.*mbr_ref3d) {
            double* data = &r3d.x;
            dist.push_back(cv::Point2d(data[plane_axes[0]], data[plane_axes[1]]));
        }
        cam.*mbr_transformMat = cv::findHomography(src, dist, 0);
    }
}

void add_points_on_interface2(
    std::vector<Cam_data>& cams,
    const std::vector<cv::Point2d> Cam_data::* mbr,
    std::vector<cv::Point3d> Cam_data::* mbr_dst,
    const cv::Mat Cam_data::* mbr_transformMat,
    const std::array<double,4> Cam_data::* mbr_interface)
{
    for (auto& cam : cams) {
        const auto& src = cam.*mbr;
        const auto& interface = cam.*mbr_interface;
        const auto& transformmat = cam.*mbr_transformMat;
        auto& interface_pts3d = cam.*mbr_dst;
        if (src.size() < 1) continue;
        std::vector<cv::Point2d> dst;
        cv::perspectiveTransform(src, dst, transformmat);
        cout << "checkpoint:787" << endl;
        for (auto dist : dst) {
            double* data = &dist.x;
            cout << "x:" << data[0] << ", \ty:" << data[1] << endl;
            std::vector<double> dst3d;
            int k = 0;
            for (int i = 0; i < 3; i++) {
                if (interface[i] < 0.000001) { dst3d.push_back(data[k]); k++; }
                else dst3d.push_back(interface[3]);
            }
            interface_pts3d.push_back(cv::Point3d(dst3d[0], dst3d[1], dst3d[2]));
        }
    }
}

std::vector<double> calcError(std::vector<cv::Point3d> tgt_p, std::vector<cv::Point3d> base_p){
    vector<vector<double>> tgt  = convert_array(tgt_p);
    vector<vector<double>> base = convert_array(base_p);
    vector<double> error, errorlist;
    for (int i = 0; i < (int)tgt.size(); i++) {
        double error_abs = 0.0;
        for (int j = 0; j < 3; j++)
            error_abs += pow((tgt[i][j+1] - base[i][j+1]), 2.0);
        errorlist.push_back(sqrt(error_abs));
    }
    auto iter = max_element(errorlist.begin(), errorlist.end());
    size_t index = std::distance(errorlist.begin(), iter);
    double mean = accumulate(errorlist.begin(), errorlist.end(), 0.0) / errorlist.size();
    error.push_back(mean);
    error.push_back(*iter);
    error.push_back((double)index);
    cout << "error_mean:" << error[0] << endl;
    cout << "error_max:"  << error[1] << endl;
    cout << "error_max_index:" << error[2] << endl;
    return error;
}

// ======================================================================
// カメラスタンバイ（08/09/11 共通）
// ======================================================================
void standbyCamera(std::vector<Cam_data>& camera, double *n, std::string dir, std::string dset_path){
    add_camID_interface(camera, dset_path);
    add_cam_intrinsics(camera, dir);
    add_2d_3d_point(camera,&Cam_data::refpoint2d, &Cam_data::refpoint3d,
                    dset_path,"reference_points_near.");
    add_2d_3d_point(camera, &Cam_data::refpoint2d_opt, &Cam_data::refpoint3d_opt,
                    dset_path,"reference_points_far.");

    add_ref_image(camera, dset_path);
    add_newCameraMatrix(camera);
    add_undistortedimages(camera);
    add_undistortedpoints(camera,&Cam_data::refpoint2d,&Cam_data::refpoint2d_u);
    add_undistortedpoints(camera,&Cam_data::refpoint2d_opt,&Cam_data::refpoint2d_opt_u);
    add_RTvec(camera);

    add_transformMat(camera,&Cam_data::transmat,
                     &Cam_data::refpoint2d_u,
                     &Cam_data::refpoint3d);
    add_points_on_interface2(camera,
                             &Cam_data::refpoint2d_opt_u,
                             &Cam_data::ref_on_interface,
                             &Cam_data::transmat,
                             &Cam_data::interface);
    add_Refracted_vector(camera,
                         &Cam_data::refpoint3d_opt,
                         &Cam_data::ref_on_interface,
                         &Cam_data::reversed_refractedRayDirs,
                         n[1],n[0]);

    estimate_reversed_focus_points(camera,
                                   &Cam_data::ref_on_interface,
                                   &Cam_data::reversed_refractedRayDirs,
                                   &Cam_data::cam_position_opt);

    add_Refracted_vector(camera,
                         &Cam_data::cam_position_opt,
                         &Cam_data::ref_on_interface,
                         &Cam_data::refractedRayDirs,
                         n[0], n[1]);

    std::vector<double> rms_list;
    estimate_all_focus_points(camera,
                                                              &Cam_data::ref_on_interface,
                                                              &Cam_data::refractedRayDirs,
                                                               &Cam_data::refpoint3d_estimated,
                                                              &rms_list);

    for (size_t j = 0; j < camera[0].refpoint3d_estimated.size(); ++j) {
        std::cout << "focus[" << j << "] = " << camera[0].refpoint3d_estimated[j]
        << " (RMS=" << rms_list[j] << ")\n";
    }

    vector<double> error=calcError(camera[0].refpoint3d_estimated,
                                   camera[0].refpoint3d_opt);
    std::vector<cv::Point3d> cam_positions;
    cam_positions.reserve(camera.size());
    for (const auto& c : camera) cam_positions.push_back(c.cam_position_opt);
    csvwriter_report(convert_array(camera[0].refpoint3d_estimated),
                     error, dset_path+"/reference_data/precision_report.csv",
                     cam_positions,
                     "Refpoint validation: ray-intersection (target-mode-independent)");
}

void csvwriter_report(vector<vector<double>> data, vector<double> data2, string filename,
                      vector<cv::Point3d> cam_positions, string mode_label){
    ofstream ofs(filename);
    if (!mode_label.empty()) {
        // mode_label は呼び出し側が "# " 以降のフルテキストを提供する
        ofs << "# " << mode_label << endl;
    }
    ofs <<"cornerNo."<<","<<"x"<<","<<"y"<<","<<"z"<<endl;
    for (int i = 0; i < (int)data.size(); i++){
        ofs << to_string(i) << ",";
        for (int j = 0; j < (int)data[0].size(); j++){
            ofs << (double)data[i][j];
            if(j != (int)data[0].size()-1) ofs << ",";
        }
        ofs << endl;
    }
    ofs << endl << "," << endl << endl;
    ofs << "mean Error" << "," << "max Error" << "," << "max Index" << endl;
    for (int i = 0; i < (int)data2.size(); i++){
        ofs << (double)data2[i] << ",";
    }
    ofs << endl << "," << endl << endl;

    // カメラ位置（cam_position_opt）— PnP/3D 推定の検証用
    if (!cam_positions.empty()) {
        ofs << "camNo.,x,y,z" << endl;
        for (int i = 0; i < (int)cam_positions.size(); i++) {
            ofs << i << ","
                << cam_positions[i].x << ","
                << cam_positions[i].y << ","
                << cam_positions[i].z << endl;
        }
        ofs << endl << "," << endl << endl;
    }
}
