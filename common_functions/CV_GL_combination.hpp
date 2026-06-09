//
//  CV_GL_combination.hpp
//  DX_studies_2025
//
//  Created by nozomi nishiumi on 2025/06/23.
//

#ifndef CV_GL_combination_hpp
#define CV_GL_combination_hpp

#include <stdio.h>
#include <vector>
#include <GL/glut.h>
#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;

void LoadGLTextures(const char* name, GLuint& texture);
void LoadCameraTextures(cv::Mat image,GLuint& texture);
void DRAW_SQU_2D(GLuint texture, double x, double y, double width, double height);
void DRAW_SQU(GLuint texture, double x, double y);
void DRAW_freeSQU(GLuint texture, vector<Point3f> p);
void setupOpenGLProjectionAndView(
    const cv::Mat& cameraMatrix,
    const cv::Mat& distCoeffs,
    const cv::Size& imageSize,
    const cv::Mat& rvec,
    const cv::Mat& tvec,
    float nearZ = 0.1f,
    float farZ = 10000.0f
                                  );

#endif /* CV_GL_combination_hpp */
