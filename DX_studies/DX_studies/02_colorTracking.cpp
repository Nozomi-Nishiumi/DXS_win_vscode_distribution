//
//  main.cpp
//  DX_studies
//
//  Created by nozomi nishiumi on 2025/06/04.
//

#include <iostream>
#include "basic_processing.hpp"
#include "OpenCV_functions.hpp"
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;
// トラックバー初期値

int main2(int argc, char *argv[])
{
    Trackbar bar;
    bar.createTrackbar("colortrack",180, 255, 255);



    Mat image=imread("../common_data/colored_triangle.png");
    while(1){
        Mat proc;
        Mat dummy=image.clone();
        colorRange(image, proc, 
                   bar.ch0_min, bar.ch0_max,
                   bar.ch1_min, bar.ch1_max,
                   bar.ch2_min, bar.ch2_max,
                   1);

        Moments oMoments = moments(proc);
        Point2d centertmp;
        centertmp.x = oMoments.m10 / oMoments.m00;
        centertmp.y = oMoments.m01 / oMoments.m00;

        cv::circle(dummy, centertmp, 20, {255,255,255},-1);

        resize(dummy, dummy, cv::Size(), 600.0/(double)dummy.cols,600.0/(double)dummy.cols);
        imshow("window",dummy);

        int key=waitKey(1);
        if(key==13){
            break;
        }
    }



    image=imread("../common_data/camera_image.png");
    Mat hsv=image.clone();
        cvtColor(image, hsv, COLOR_BGR2HSV);

    while(1){
        Mat proc;
        colorRange(hsv, proc,
                   bar.ch0_min, bar.ch0_max,
                   bar.ch1_min, bar.ch1_max,
                   bar.ch2_min, bar.ch2_max,
                   1);
        Moments oMoments = moments(proc);
        Point2d centertmp;
        centertmp.x = oMoments.m10 / oMoments.m00;
        centertmp.y = oMoments.m01 / oMoments.m00;

        Mat dummy=image.clone();
        cv::circle(dummy, centertmp, 20, {255,255,255},-1);

        resize(dummy, dummy, cv::Size(), 600.0/(double)dummy.cols,600.0/(double)dummy.cols);
        imshow("window",dummy);

        int key=waitKey(1);
        if(key==13){
            break;
        }
    }

    
//    Point2d blue= colorTrack(hsv, 0, 255, 0, 255, 0, 255,1);
//    Point2d green=colorTrack(hsv,  0,  255, 0, 255, 0, 255,1);
//    Point2d red=  colorTrack(hsv, 0,   255, 0, 255, 0, 255,1);
//
//    cv::circle(image, blue,  20, {255,255,255},-1);
//    cv::circle(image, green, 20, {255,255,255},-1);
//    cv::circle(image, red,   20, {255,255,255},-1);
//
//    resize(image, image, cv::Size(), 600.0/(double)image.cols,600.0/(double)image.cols);
//    imshow("window",image);
//    while(1){
//        int key=waitKey(1);
//        if(key==13){
//            break;
//        }
//    }
    return(0);
}
