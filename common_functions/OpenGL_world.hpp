//
//  opengl_world.hpp
//  opengl_class_study
//
//  Created by nozomi nishiumi on 2025/05/06.
//

#ifndef opengl_world_hpp
#define opengl_world_hpp

#include <functional>
#include <vector>
#include <GL/glut.h>
#include <glm.hpp>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>

using namespace std;

//void loadOBJ(std::string tgtOBJ,vector<GLMmodel*>&  models, vector<GLuint>& model_lists);

class MyGLApp {
public:
    static MyGLApp& getInstance();
    double POVfrom[3],Head[3],POVto[3];
    double vangle=60.0;
    // 'v' キーで切り替える視点モード（false: 通常視点 / true: カメラ姿勢ベースの viewsetting2）。
    // 描画側(各モード)がこのフラグを見て viewsetting() / viewsetting2() を選ぶ。
    bool viewmode2 = false;

    void init(int argc, char** argv);
    void run();

    // ユーザー定義の display 関数を登録する
    void setDisplayFunction(const std::function<void()>& func);
    void setAddDisplayFunction(const std::function<void()>& func);
    void setWindowSize(int width, int height);


    static void navigation_guidance_2_wrapper(std::vector<std::vector<double>> &dx,
                                            std::vector<std::vector<double>> &x);
private:
    MyGLApp();
    int windowWidth = 400;
    int windowHeight = 800;
    std::vector<double> obj={0.0,-600.0,200.0,0.0,0.0,0.0,0.0,0.0};
    std::vector<std::vector<double>> casts={obj};
    std::vector<double> Camera_head={0,0,0};
    const double pi = 3.14159265358979323846264338327950288;
    float rot[4]={0.0,0.0,0.0,0.0};
    double steer[4]={0.0,0.0,0.0,0.0};
    enum {Camera=0};
    enum {X=0,Y=1,Z=2,hLOS=3,vLOS=4,hHEAD=5,vHEAD=6,velocity=7};

    int WindowPositionX = 0;  //生成するウィンドウ位置のX座標
    int WindowPositionY = 0;  //生成するウィンドウ位置のY座標
    int WindowWidth;   //生成するウィンドウの幅
    int WindowHeight;    //生成するウィンドウの高さ
    int MainWinID[2];

    bool steerR, steerL, steerU, steerD,zzz,xxx,aaa,sss;
    double Vmax=3000*0.6;
    const double turnlimit = 1.0;
    vector<vector<double>> body,head;
//    double POVfrom[3],Head[3],POVto[3];
    int tgtcast,tgtlock=0;
    int elapsed_time = 0;
    GLuint texture[1];
    GLuint GL_texture[1];
    GLMmodel* model = nullptr;

    void pramInit();
    void steering2();
    void ODE_evolve_2(const double t,
                      const double dt,
                      vector<vector<double>> &x,
                      void(*ode)(
                                 vector<vector<double>> &,
                                 vector<vector<double>> &)
                      );
    void pilotControl_P_2(vector<double> &dx, vector<double> &x, double steering[4]);
    void navigation_guidance_2(vector<vector<double>> &dx,
                               vector<vector<double>> &x);


    void update_visualObj();



    //コールバック系
    static void displayCallback();
    static void idleCallback();
    static void keyboardCallback(unsigned char key, int x, int y);
    static void keyboardUpCallback(unsigned char key, int x, int y);
    static void keyboardSpecCallback(int key, int x, int y);
    static void keyboardSpecUpCallback(int key, int x, int y);

    void display();
    void idle();
    void key_down(int key, int x, int y);
    void key_up(int key, int x, int y);
    void key_down_nml(unsigned char key, int x, int y);
    void key_up_nml(unsigned char key, int x, int y);

    void param();
    void normalize();

    void keyboard(unsigned char key, int x, int y);

    std::function<void()> userDisplayFunc; // ユーザーがセットする描画関数
    std::function<void()> userAddDisplayFunc; // ユーザーがデフォルトに追加する描画関数
    std::function<void()> defaultDisplayFunc();
    std::function<void()> defaultAddDisplayFunc();
    std::function<void()> defaultDisplayFunc2();

};

struct FBO {
    GLuint fbo = 0;
    GLuint colorTex = 0;
    GLuint depthRbo = 0;
    int width = 0;
    int height = 0;
};
bool CreateFBO(FBO& outFBO, int width, int height);
void BeginFBO(const FBO& fbo);
void EndFBO();
void DrawFBO_Additive(const FBO& fbo);
void DeleteFBO(FBO& fbo);



struct Vec3 {
    double x, y, z;
};
Vec3 getBoundingBoxSize(GLMmodel* model);



void drawGround(const vector<double> eye,int order,double R, double G, double B);

void glmTransform(GLMmodel* model, GLfloat matrix[4][4]);
void glmRotateModel(GLMmodel* model, float angleDeg, float x, float y, float z);
void glmTranslateModel(GLMmodel* model, float dx, float dy, float dz);
void glmBoundingBox(GLMmodel* model, float min[3], float max[3]);
void makeShadowMatrix(GLfloat shadowMat[16], const GLfloat light[4], const GLfloat plane[4]);
void drawSoftShadow(const GLfloat light[4], const GLfloat plane[4], void (*drawObject)());

// 共通グローバル（シングルトン参照とフレームカウンタ）
extern MyGLApp& app_gl;
extern int frame_gl;

// 共通ビュー設定（07/08/09/11 共通）
void viewsetting();

// OBJファイル読み込みユーティリティ
void loadOBJ(GLMmodel*& model, const std::string& filepath);

#endif
