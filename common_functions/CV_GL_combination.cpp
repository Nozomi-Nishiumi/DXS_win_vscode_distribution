//
//  CV_GL_combination.cpp
//  DX_studies_2025
//
//  Created by nozomi nishiumi on 2025/06/23.
//
#define GL_SILENCE_DEPRECATION

#include "CV_GL_combination.hpp"


void LoadGLTextures(const char* name, GLuint& texture)
{
    cv::Mat image= cv::imread(name, -1);
    cv::Mat tex;
    int colormode;
    if(image.channels()==1){
        tex=image;
        colormode=GL_LUMINANCE;
    }else{
        cvtColor(image, tex, cv::COLOR_BGRA2RGBA);
        colormode=GL_RGBA;

    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.cols, tex.rows, 0, colormode, GL_UNSIGNED_BYTE, tex.data);

};


void LoadCameraTextures(cv::Mat image,GLuint& texture)
{

    cv::Mat tex_cam;
    int colormode;
    if(image.channels()==1){
        tex_cam=image;
        colormode=GL_LUMINANCE;
    }else{
        cvtColor(image, tex_cam, cv::COLOR_BGR2RGBA);
        colormode=GL_RGBA;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_cam.cols, tex_cam.rows, 0, colormode, GL_UNSIGNED_BYTE, tex_cam.data);
}

void DRAW_SQU_2D(GLuint texture, double x, double y, double width, double height)
{
    // ① 射影・モデルビュー行列を保存
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // ② ウィンドウ座標に合わせた正射影を設定（左上を(0,0)にするなら調整も可能）
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);  // [x, y, width, height]
    gluOrtho2D(0, viewport[2], viewport[3], 0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();


    glPushMatrix();

    // ③ テクスチャ有効化・設定
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, texture);

    // ④ ピクセル単位の四角形を描画（左下基準）
    glBegin(GL_QUADS);
        glTexCoord2d(0.0, 0.0); glVertex2d(x,         y);
        glTexCoord2d(1.0, 0.0); glVertex2d(x + width, y);
        glTexCoord2d(1.0, 1.0); glVertex2d(x + width, y + height);
        glTexCoord2d(0.0, 1.0); glVertex2d(x,         y + height);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glPopMatrix();
    // ⑤ 行列を元に戻す
    glPopMatrix(); // modelview
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}



void DRAW_SQU(GLuint texture, double x, double y)
{
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLint w=x;
    GLint h=y;
    //    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    //    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

    glBegin(GL_QUADS);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(w,0,0);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(w,h,0);
    glTexCoord2d(0.0,0.0);
    glVertex3d(0,h,0);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(0,0,0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glPopMatrix();


}


void DRAW_freeSQU(GLuint texture, vector<Point3f> p)
{
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    glEnable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, texture);


    glBegin(GL_QUADS);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(p[3].x,p[3].y,p[3].z);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(p[1].x,p[1].y,p[1].z);
    glTexCoord2d(0.0,0.0);
    glVertex3d(p[0].x,p[0].y,p[0].z);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(p[2].x,p[2].y,p[2].z);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glPopMatrix();


}

void setupOpenGLProjectionAndView(
    const cv::Mat& cameraMatrix,
    const cv::Mat& distCoeffs,
    const cv::Size& imageSize,
    const cv::Mat& rvec,
    const cv::Mat& tvec,
    float nearZ,
    float farZ
) {
    // 無効なカメラ行列(空/非3x3)では undistortPoints が assertion で terminate する。
    // 呼び出し側が共有Matをスレッド越しに渡してくる場合に一時的な不整合が起こりうるため、
    // ここで弾いて投影設定をスキップする(そのフレームは前フレームの投影を維持)。
    if(cameraMatrix.empty() || cameraMatrix.rows!=3 || cameraMatrix.cols!=3) return;

    // 1. OpenGL投影行列を作成（glFrustum）

    // 画像の4隅
    std::vector<cv::Point2f> imgCorners = {
        {0, 0},
        {(float)imageSize.width, 0},
        {(float)imageSize.width, (float)imageSize.height},
        {0, (float)imageSize.height}
    };

    // 歪み補正 & 正規化座標（焦点距離=1のカメラの画像平面上）
    std::vector<cv::Point2f> normPoints;
    cv::undistortPoints(imgCorners, normPoints, cameraMatrix, distCoeffs);

    // Near面の投影端
    float left   = normPoints[0].x * nearZ;
    float right  = normPoints[1].x * nearZ;
    float top    = normPoints[0].y * nearZ;
    float bottom = normPoints[2].y * nearZ;

    // OpenGL投影行列をセット
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(left, right, bottom, top, nearZ, farZ);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    cv::Mat R;
    cv::Rodrigues(rvec, R);
    cv::Mat R_inv = R.t(); // 転置＝逆回転
    cv::Mat t_inv = -R_inv * tvec;
    cv::Vec3d axis_angle;
    cv::Rodrigues(R_inv, axis_angle);

    double angle_rad = cv::norm(axis_angle);
    if (angle_rad > 1e-6) {
        cv::Vec3d axis = axis_angle / angle_rad;
        glScaled(1,1,-1);
        glRotated((-angle_rad * 180.0 / CV_PI), axis[0], axis[1], axis[2]);
    }
    glTranslated(-t_inv.at<double>(0), -t_inv.at<double>(1), -t_inv.at<double>(2));

}
