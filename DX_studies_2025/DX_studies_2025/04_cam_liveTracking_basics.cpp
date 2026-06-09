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



int main4() {
    // 現在の実行ディレクトリをプリント
    std::cout << "実行時ディレクトリ: " << fs::current_path() << std::endl;
    
    Trackbar bar;
    bar.createTrackbar("camera_track",180,255,255);

    cv::VideoCapture cap=cv::VideoCapture(0);
    cap.set(CAP_PROP_FRAME_WIDTH, 800);
    cap.set(CAP_PROP_FRAME_HEIGHT, 600);
    
    while(1){
        
        Mat image;
        cap>>image;
        
        if(!image.empty()){
            Mat hsv=image.clone();
            cvtColor(image, hsv, COLOR_BGR2HSV);
            Point2d centertmp= colorTrack(hsv,
                                     bar.ch0_min, bar.ch0_max,
                                     bar.ch1_min, bar.ch1_max,
                                     bar.ch2_min, bar.ch2_max,
                                     1);
            int size=100;
            Rect tgt(centertmp.x-size,centertmp.y-size,2*size,2*size);
            cv::rectangle(image, tgt,{255,255,255},3);

            //
            resize(image, image, cv::Size(), 600.0/(double)image.cols,600.0/(double)image.cols);
            cv::imshow("live image", image);
            int key=waitKey(1);
            if(key==13){
                break;
            }
        }
    }
    
//    while(1){
//        
//        Mat image;
//        cap>>image;
//        
//        if(!image.empty()){
//            Mat hsv=image.clone();
//            cvtColor(image, hsv, COLOR_BGR2HSV);
//            Point2d blue= colorTrack(hsv, 0, 255, 0, 255, 0, 255,1);
//            Point2d green=colorTrack(hsv,  0,  255, 0, 255, 0, 255,1);
//            Point2d red=  colorTrack(hsv, 0,   255, 0, 255, 0, 255,1);
//            
//            cv::circle(image, green, 20, {255,255,255},-1);
//            cv::circle(image, red, 20, {255,255,255},-1);
//            cv::circle(image, blue, 20, {255,255,255},-1);
//            
//            resize(image, image, cv::Size(), 600.0/(double)image.cols,600.0/(double)image.cols);
//            cv::imshow("live image", image);
//            
//            int key=waitKey(1);
//            if(key==13){
//                break;
//            }
//        }
//    }
    
    return 0;
}
