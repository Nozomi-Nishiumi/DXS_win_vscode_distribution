#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "basic_processing.hpp"
#include "OpenCV_functions.hpp"
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

using namespace cv;
using namespace std;




int main5() {

    // 現在の実行ディレクトリをプリント
    std::cout << "実行時ディレクトリ: " << fs::current_path() << std::endl;

    Cam_data cam("cam0");   
    cam.ori.createTrackbar(180,255,255);
    cam.oth.createTrackbar(180,255,255);
    cam.tgt.createTrackbar(180,255,255);

    cv::VideoCapture cap=cv::VideoCapture(0);
    cap.set(CAP_PROP_FRAME_WIDTH, 800);
    cap.set(CAP_PROP_FRAME_HEIGHT, 600);


    while(1){

        Mat image;
        cap>>image;

        if(!image.empty()){


            Mat hsv=image.clone();
            cvtColor(image, hsv, COLOR_BGR2HSV);
            Mat proc;
            colorRange(hsv,proc,
                       cam.ori.ch0_min, cam.ori.ch0_max,
                       cam.ori.ch1_min, cam.ori.ch1_max,
                       cam.ori.ch2_min, cam.ori.ch2_max,
                       1);

            vector<Point2f> origin=get_N_targetpoints(proc, 1);

            colorRange(hsv,proc,
                       cam.oth.ch0_min, cam.oth.ch0_max,
                       cam.oth.ch1_min, cam.oth.ch1_max,
                       cam.oth.ch2_min, cam.oth.ch2_max,
                       1);

            vector<Point2f> others=get_N_targetpoints(proc, 3);

            colorRange(hsv,proc,
                       cam.tgt.ch0_min, cam.tgt.ch0_max,
                       cam.tgt.ch1_min, cam.tgt.ch1_max,
                       cam.tgt.ch2_min, cam.tgt.ch2_max,
                       1);

            vector<Point2f> tgts=get_N_targetpoints(proc, cam.tgt.N);
            // tgt も未検出ぶんを (0,0) で補完し、未検出でもシーカーを常時表示する。
            while ((int)tgts.size() < cam.tgt.N) tgts.push_back(Point2f(0.f, 0.f));

            vector<Point2f> merged;
            merged.reserve(origin.size() + others.size());
            merged.insert(merged.end(), origin.begin(), origin.end());
            merged.insert(merged.end(), others.begin(), others.end());

            vector<Point3f> objectPoints = {
                Point3f(0.0f, 0.0f, 0.0f),
                Point3f(1.0f, 0.0f, 0.0f),
                Point3f(0.0f, 1.0f, 0.0f),
                Point3f(1.0f, 1.0f, 0.0f)
            };

            Scalar col_ori=averageHSV(
                                        Scalar(cam.ori.ch0_min,
                                               cam.ori.ch1_min,
                                               cam.ori.ch2_min),
                                        Scalar(cam.ori.ch0_max,
                                               cam.ori.ch1_max,
                                               cam.ori.ch2_max));

            Scalar col_oth=averageHSV(
                                        Scalar(cam.oth.ch0_min,
                                               cam.oth.ch1_min,
                                               cam.oth.ch2_min),
                                        Scalar(cam.oth.ch0_max,
                                               cam.oth.ch1_max,
                                               cam.oth.ch2_max));

            Scalar col_tgt=averageHSV(
                                        Scalar(cam.tgt.ch0_min,
                                               cam.tgt.ch1_min,
                                               cam.tgt.ch2_min),
                                        Scalar(cam.tgt.ch0_max,
                                               cam.tgt.ch1_max,
                                               cam.tgt.ch2_max));


            int size=100;

            // get_N_targetpoints は実際に検出できた点しか返さない（未検出ぶんを (0,0) 等で補完しない）。
            // 一方 matchCorrespondingPoints はちょうど4点を要求するため、未検出フレームでは
            // 点数不足のまま CV_Assert が例外を投げてクラッシュする。
            // 従前の挙動（未検出マーカーは原点(0,0)にシーカー表示）を復元するため、
            // 不足ぶんを (0,0) で補って常に4点にし、未検出でも必ず描画されるようにする。
            while (merged.size() < 4) merged.push_back(Point2f(0.f, 0.f));
            if (merged.size() > 4) merged.resize(4);

            auto [matched2D, matched3D] = matchCorrespondingPoints(merged, objectPoints);

            for(int i=0;i<matched2D.size();i++){
                Scalar col;
                if(i<1){
                    col=col_ori;
                }else{
                    col=col_oth;
                }

                string text=to_string(i)/*+":"+to_string((int)matched3D[i].x)+","+to_string((int)matched3D[i].y)*/;
                putText(image, text, Point2i(matched2D[i].x-size,matched2D[i].y-size-5), FONT_HERSHEY_DUPLEX, 2, col,5);
                Rect tgt(matched2D[i].x-size,matched2D[i].y-size,
                         2*size,2*size);
                cv::rectangle(image, tgt,col,20);
            }

            for(int i=0;i<tgts.size();i++){

                string text="TGT_"+to_string(i);
                putText(image, text, Point2i(tgts[i].x-size,tgts[i].y-size-5), FONT_HERSHEY_DUPLEX, 2, col_tgt,5);
                Rect tgt(tgts[i].x-size,tgts[i].y-size,
                         2*size,2*size);
                cv::rectangle(image, tgt,col_tgt,20);
            }

            resize(image, image, cv::Size(), 600.0/(double)image.cols,600.0/(double)image.cols);            cv::imshow("live image", image);


            int key=waitKey(1);
            if(key==13){
                break;
            }
        }
    }



    return 0;
}

