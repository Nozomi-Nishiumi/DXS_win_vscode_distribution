#define GL_SILENCE_DEPRECATION
#define lego 8.0
// Windows(MSYS2)対策: GL/glut.h 経由の <windows.h> を、OpenCV(using namespace cv)より
// 先に読み込む。逆順だと cv::ACCESS_MASK と winnt.h の ACCESS_MASK が衝突してビルド不可。
#include "OpenGL_world.hpp"
#include "OpenCV_functions.hpp"
#include "CV_GL_combination.hpp"
#include <thread>
#include <chrono>
#include <fstream>
#include <json.hpp>
#include <filesystem>
using namespace std;
using namespace cv;

namespace {
//vector<GLMmodel*>  models;
//vector<GLuint> model_lists;
GLMmodel* models[]{};
vector<double> obj_7={0.0,0.0,50.0,0.0,0.0,0.0,0.0,0.0};

Cam_data camera("cam0");

Point3f cam;
vector<Point2f> impoint2d;
vector<Point3f> impoint3d;
vector<Point3f> transport_3d={cv::Point3f(0,0,0)};
vector<Point3f> trackdata_3d={cv::Point3f(0,0,0)};
vector<Mat> cameraData;
GLuint texture_7[2]={0,1};
GLuint GL_texture_7[1];

// --- 2個目のtgt: ArUcoマーカーの姿勢(位置+向き)をワールド座標系で保持 -------------
// 1個目のtgt(色追跡)は「方向」しか得られないので z=0平面との交点で位置を決めるが、
// ArUcoは4隅が既知なので solvePnP で「位置」と「姿勢(回転)」の両方を復元できる。
// ArUcoはIDを最初から返す(辞書DICT_4X4_50, ID 0〜49)ので、ID毎に別モデルを表示する。
struct ArucoPose {
    bool    found=false;
    Point3f pos=Point3f(0,0,0); // マーカー中心のワールド座標
    Vec3d   axis=Vec3d(0,0,1);  // glRotated 用の回転軸
    double  angle_deg=0.0;      // glRotated 用の回転角[deg]
    int     miss=0;             // 連続で見失った検出回数(ヒステリシス用)
};
const int ARUCO_MAX_ID=4;          // 認識するマーカーのID範囲 [0, ARUCO_MAX_ID)
// 検出は毎回100%成功しないため、見失っても直近の姿勢をこの回数だけ保持してCGの
// 点滅を防ぐ。大きいほど点滅は減るが、実際に外した後も残像が長く残る。
const int ARUCO_MISS_LIMIT=8;
ArucoPose aruco_poses[ARUCO_MAX_ID]; // aruco_tracking() が書き込み、draw_CGmodels_aruco() が参照
// マーカーのID → 表示するCGモデル番号(各自編集可)。未ロードの番号は描画されない。
const int aruco_model_of_id[ARUCO_MAX_ID]={1, 0, 1, 0};
// ArUcoマーカーの実寸(1辺=黒枠込み)。calib() の objectPoints と同じ単位(=lego単位)で、
// 印刷したマーカーの実サイズに合わせて調整する。位置・姿勢の絶対スケールはこの値で決まる。
const double aruco_marker_size=18.8;





// 実ウィンドウに、カメラのアスペクトを保って収まるビューポート矩形(中央フィット)を計算。
// 従来の glViewport(0,0, aspect*800, 800) は実ウィンドウ寸法(init: 1280x832)と無関係な
// ハードコードで、右切れ・上隙間の原因だった。実ウィンドウを glutGet で取得して合わせる。
void cameraFitViewport(int& vx, int& vy, int& vw, int& vh){
    int ww = glutGet(GLUT_WINDOW_WIDTH), wh = glutGet(GLUT_WINDOW_HEIGHT);
    if(ww<=0||wh<=0){ vx=vy=0; vw=ww; vh=wh; return; }
    double a = (camera.im_ori.empty())
               ? (16.0/9.0)
               : (double)camera.im_ori.cols/(double)camera.im_ori.rows;
    if ((double)ww/(double)wh > a) { vh = wh; vw = (int)(a*wh); }   // 窓が横長 → 高さ基準
    else                          { vw = ww; vh = (int)(ww/a);  }   // 窓が縦長 → 幅基準
    vx = (ww-vw)/2; vy = (wh-vh)/2;
}

void viewsetting2(){
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //バッファの消去

    // 共有Mat(cameraData/rvec/tvec)を局所にスナップショットして固定し、有効なときだけ投影。
    // calibスレッドが cameraData を再代入する瞬間に読むと空/非3x3になり、setupOpenGL…内の
    // undistortPoints が terminate する（今回の強制終了の経路）。
    if(cameraData.size()<2) return;
    cv::Mat K=cameraData[0], D=cameraData[1], rv=camera.rvec, tv=camera.tvec;
    if(K.empty()||K.rows!=3||K.cols!=3||rv.empty()||tv.empty()) return;
    setupOpenGLProjectionAndView(K, D, Size(camera.im_ori.cols,camera.im_ori.rows), rv, tv);
    // フラスタム底面(カメラ画像)が実ウィンドウに切れず・歪まず収まるよう中央フィット。
    int vx,vy,vw,vh; cameraFitViewport(vx,vy,vw,vh);
    glViewport(vx, vy, vw, vh);
}

string round_str(vector<double> values, int precision){
    string result="";
    for(auto v:values){
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << v;
        result=result+oss.str()+". ";
    }
    return result;
}

void draw_environment(){

    vector<Point3f> frustum,frustum_ori;
    frustum.insert(frustum.end(), transport_3d.begin() + 0, transport_3d.begin() + 4);
    frustum_ori.insert(frustum_ori.end(), transport_3d.begin() + 4, transport_3d.begin() + 8);

    if(camera.capsize.width > 0 && camera.capsize.height > 0)
        DRAW_SQU_2D(texture_7[0],0,0,(double)camera.capsize.width/(double)camera.capsize.height*300.0, 300);

    DRAW_freeSQU(texture_7[0],{
        frustum[0],
        frustum[1],
        frustum[3],
        frustum[2],
    });

//    DRAW_SQU(texture_7[1], 1000, 600);


    glColor3d(1, 1, 1);
    glPointSize(10);
    glBegin(GL_LINE_LOOP);
    for(const auto& obj:frustum){
        glVertex3d(obj.x,obj.y,obj.z);
    }
    glEnd();


    if(!camera.rvec.empty()&&!camera.tvec.empty()){
        glPushMatrix();
        glPushAttrib(GL_CURRENT_BIT);
        cv::Mat R;
        cv::Rodrigues(camera.rvec, R);
        cv::Mat R_inv = R.t();
        cv::Mat t_inv = -R_inv * camera.tvec;

        cv::Vec3d axis_angle;
        cv::Rodrigues(R_inv, axis_angle);

        glTranslated(t_inv.at<double>(0), t_inv.at<double>(1), t_inv.at<double>(2));

        double angle_rad = cv::norm(axis_angle);
        if (angle_rad > 1e-6) {
            cv::Vec3d axis = axis_angle / angle_rad;
            glRotated((angle_rad * 180.0 / CV_PI), axis[0], axis[1], axis[2]);
        }


        glColor3d(1, 0.5, 0.5);
        glBegin(GL_LINE_LOOP);
        for(const auto& obj:frustum_ori){
            glVertex3d(obj.x,obj.y,obj.z);
        }
        glEnd();

        glBegin(GL_LINES);
        for(const auto& obj:frustum_ori){
            glVertex3d(obj.x,obj.y,obj.z);
            glVertex3d(0,0,0);
        }
        glEnd();

        glPopMatrix();
        glPopAttrib();
    }


    glBegin(GL_POINTS);
    for(const auto& obj:frustum){
        glVertex3d(obj.x,obj.y,obj.z);
    }
    glVertex3d(cam.x,cam.y,0.0);
    glVertex3d(cam.x,cam.y,cam.z);

    glEnd();



    glBegin(GL_LINES);
    glVertex3d(cam.x,cam.y,cam.z);
    glVertex3d(cam.x,cam.y,0.0);
    glEnd();

    vector<Point3f> fov=intersect(cam,frustum,0);
    glDisable(GL_DEPTH_TEST);
    glBegin(GL_LINE_LOOP);
    for(const auto& obj:fov){
        glVertex3d(obj.x,obj.y,obj.z);
    }
    glEnd();
    glEnable(GL_DEPTH_TEST);




    glPushMatrix();
    glDisable(GL_DEPTH_TEST);
    glColor3d(1,1,1);
    for(int i=0;i<impoint3d.size();i++){
        glRasterPos3f(impoint3d[i].x,impoint3d[i].y,impoint3d[i].z);
        string text=round_str({impoint3d[i].x,impoint3d[i].y,impoint3d[i].z}, 0);
        for(char c:text){
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,c);
        }
    }
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();

    glPushMatrix();
    glPointSize(3);
    glBegin(GL_POINTS);
    for(const auto& obj:impoint3d){
        glVertex3d(obj.x,obj.y,obj.z);
    }
    glEnd();

    glBegin(GL_LINES);
    for(const auto& obj:impoint3d){
        glVertex3d(cam.x,cam.y,cam.z);
        glVertex3d(obj.x,obj.y,obj.z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glDisable(GL_DEPTH_TEST);
    glColor3d(1,1,1);
        glRasterPos3f(cam.x,cam.y,cam.z);
    
    string text=round_str({cam.x,cam.y,cam.z},0);
    for(char c:text){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,c);
    }
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();


    //陰影ON-----------------------------
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);//光源0を利用

}

void draw_CGmodels(){
    if (models[0]&&trackdata_3d.size()>0&&
        cam.x!=0&&cam.y!=0&&cam.z!=0&&
        trackdata_3d.size()>0){
        vector<Point3f> tgt=intersect(cam,trackdata_3d,0);

        for(auto loc:tgt){
            glPushMatrix();
            glTranslated(loc.x, loc.y, loc.z);
            glmDraw(models[0], GLM_SMOOTH | GLM_COLOR);
            glPopMatrix();
        }
}

}

void draw_GhostCGmodels(){
    if (models[0]&&trackdata_3d.size()>0&&
        cam.x!=0&&cam.y!=0&&cam.z!=0&&
        trackdata_3d.size()>0){
        vector<Point3f> tgt=intersect(cam,trackdata_3d,0);

        glDisable(GL_LIGHTING);
        glPushMatrix();
        glTranslated(tgt[0].x, tgt[0].y, tgt[0].z);
        glColor4d(0, 0.3, 0, 0.05);
        glmDraw(models[0], GLM_SMOOTH);
        glPopMatrix();
}
}

// 2個目のtgt(ArUco)を、検出されたIDごとに対応モデルで表示。
// aruco_tracking() がそのIDの姿勢を確定したフレームでのみ描画する。
void draw_CGmodels_aruco(){
    for(int id=0; id<ARUCO_MAX_ID; id++){
        if(!aruco_poses[id].found) continue;
        int m=aruco_model_of_id[id];
        if(m<0 || !models[m]) continue;
        glPushMatrix();
        glTranslated(aruco_poses[id].pos.x, aruco_poses[id].pos.y, aruco_poses[id].pos.z); // 中心へ
        if(aruco_poses[id].angle_deg>1e-6)                                                 // 向きへ
            glRotated(aruco_poses[id].angle_deg,
                      aruco_poses[id].axis[0], aruco_poses[id].axis[1], aruco_poses[id].axis[2]);
        glmDraw(models[m], GLM_SMOOTH | GLM_COLOR);
        glPopMatrix();
    }
}

void draw_simplecg(){
    float time_s = glutGet(GLUT_ELAPSED_TIME)/1000.0;
    glPushMatrix();
    glTranslated(time_s*5.0, 0, time_s*5.0);
    glmDraw(models[0], GLM_SMOOTH);
    glPopMatrix();
}

void FBO_contents(){
    // myDisplay_7 と同じ視点選択ロジックに統一（'v' = MyGLApp::viewmode2）。
    // viewsetting2 は姿勢(rvec/tvec)・cameraData・映像が必要で、未確定のまま呼ぶと空行列で落ちる。
    if(app_gl.viewmode2){
        if(!camera.im_ori.empty() && cameraData.size()>=2
           && !camera.rvec.empty() && !camera.tvec.empty()){
            viewsetting2();                   // 姿勢が揃っている → 視点2
        }else{
            // 姿勢等が未確定：ユーザー選択の視点を勝手に視点1へ戻さない（投影は維持）。
            // バッファだけ消去してエラー/残像を回避。
            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
    }else{
        viewsetting();                        // ユーザーが視点1を選択中
    }



//     float time_s = glutGet(GLUT_ELAPSED_TIME)/1000.0;
//     glEnable(GL_BLEND);
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


//     glDepthMask(GL_FALSE);  // 深度バッファ書き込みOFF

//     glDisable(GL_LIGHTING);
//     GLfloat light_pos[] = {0,0,100.0f,0};
//     GLfloat plane[4] = {0.0f, 0.0f, 1.0f, 0.0f};
//     drawSoftShadow(light_pos, plane, draw_simplecg);

//     glDepthMask(GL_TRUE);   // 書き込みONに戻す
//  glEnable(GL_BLEND);
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


//     glEnable(GL_LIGHTING);
//     glEnable(GL_LIGHT0);
//     glPushMatrix();
//     glTranslated(time_s*5.0, 0, time_s*5.0);
//     glmDraw(models[0], GLM_SMOOTH| GLM_COLOR);
//     glPopMatrix();



    // glmDraw(models[1], GLM_SMOOTH);

    // draw_CGmodels();      // 1個目のtgt(色追跡): z=0交点に models[0]
    // draw_CGmodels_aruco();// 2個目のtgt(ArUco): IDごとの位置・姿勢に対応モデル

}


void FBO_layer(){
    glClear(GL_DEPTH_BUFFER_BIT);
//    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    FBO fboObj2;
    // FBO を実ウィンドウサイズで作り、合成も 1:1(0,0,ww,wh) で行う。こうすると FBO 内で
    // viewsetting2/viewsetting が設定するビューポート位置が、そのままウィンドウ上の位置と
    // 一致するため、背景(myDisplay_7 で描画)と CG レイヤがズレない。
    int ww = glutGet(GLUT_WINDOW_WIDTH), wh = glutGet(GLUT_WINDOW_HEIGHT);
    if(ww<=0) ww=1;
    if(wh<=0) wh=1;
    CreateFBO(fboObj2, ww, wh);
    FBO_contents();
    EndFBO();
    glViewport(0, 0, ww, wh);
    DrawFBO_Additive(fboObj2);
    DeleteFBO(fboObj2);
}

void occulusion_viewer(){
    glClear(GL_DEPTH_BUFFER_BIT);                        // occluderを wall だけにする（色は残すので背景/カメラ画像はそのまま）
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // 色出力オフ
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_TRUE);                                // 深度は書き込む
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glPushMatrix();
    glTranslated(0, -100, 0);
    glmDraw(models[1], GLM_SMOOTH| GLM_COLOR);
    glPopMatrix();
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // 色出力オン
    glDepthFunc(GL_GREATER); // aより奥の部分だけ通過

    draw_GhostCGmodels();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    glTranslated(0, -100, 0);
    glmDraw(models[1], GLM_SMOOTH| GLM_COLOR);
    glPopMatrix();
    glDepthFunc(GL_LESS);   // GL_GREATER を既定へ戻す（後続/次フレームへの状態リーク防止）
    glEnable(GL_DEPTH_TEST);
}

void myDisplay_7() {
    frame_gl++;//フレームカウント




    if(!camera.im_ori.empty()){
        Mat txtr;
//        undistort(camera.im_ori, txtr, camera.cam_mat, camera.dist_coefs, camera.new_cam_mat);

        //        for(int i=0;i<impoint2d.size();i++){
//            cv::circle(txtr, impoint2d[i], 50,Scalar(255,255,255),7);
//            cv::putText(txtr,to_string(i), impoint2d[i],FONT_HERSHEY_DUPLEX, 5,Scalar(255,255,255));
//        }
        LoadCameraTextures(camera.im_ori, texture_7[0]);
    }


    // 'v' キーで viewsetting()⇔viewsetting2() を切替（MyGLApp::viewmode2 がトグルされる）。
    // viewsetting2 はカメラ姿勢(rvec/tvec)・cameraData・映像が必要で、未確定のまま呼ぶと
    // 空行列演算で cv::Exception になる。
    if(app_gl.viewmode2){
        if(!camera.im_ori.empty() && cameraData.size()>=2
           && !camera.rvec.empty() && !camera.tvec.empty()){
            viewsetting2();                   // 姿勢が揃っている → 視点2
        }else{
            // 姿勢等が未確定：ユーザー選択の視点を勝手に視点1へ戻さない（投影は維持）。
            // display() はクリアしないので、バッファだけ消去してエラー/残像を回避。
            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
    }else{
        viewsetting();                        // ユーザーが視点1を選択中
    }


    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);

    drawGround(vector<double> {
        app_gl.POVfrom[0],
        app_gl.POVfrom[1],
        app_gl.POVfrom[2]},
               100,0.4,0.4,0.4);


    // 背景を描画
//    glDisable(GL_DEPTH_TEST);
    draw_environment();
//    glEnable(GL_DEPTH_TEST);


//    occulusion_viewer();
   FBO_layer();

}

void additionalDisplay_7() {

}











void cap(){

    int camnum=0;
    while(1){
        if(VideoCapture(camnum).isOpened()){
            cout<<"cam"<<to_string(camnum)<<":Found"<<endl;
            camnum++;
        }else{
            cout<<"cam"<<to_string(camnum)<<":None"<<endl;
            break;
        }
    }

    //カメラキャプチャの初期化
    VideoCapture capture(0);
    if (!capture.isOpened() ) {
        cerr<<"Camera not found\n"<<endl;
        exit(0);
    }

    capture.set(CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(CAP_PROP_FRAME_HEIGHT, 720);


    while (1){
        capture >> camera.im_ori;
        if(!camera.im_ori.empty()){
            camera.capsize.width=camera.im_ori.cols;
            camera.capsize.height=camera.im_ori.rows;
            break;
        }
    }


    while (1){
        capture >> camera.im_ori;
    }

}

void tgt_tracking(){
//    camera.tgt.loadparameters("tgt");

    // string yml="./output/cam_intrinsic_prameters_new.yml";


    // cv::FileStorage fs(yml, cv::FileStorage::READ);
    // fs["cameraMatrix"] >> camera.cam_mat;
    // fs["distCoeffs"] >> camera.dist_coefs;
    // fs.release();
    while(1){
        if(camera.im_ori.empty()||(camera.rvec.rows+camera.rvec.cols)!=4){continue;}
        Mat src=camera.im_ori.clone();
        // 新しいカメラ行列（必要に応じて画像サイズやスケーリング係数を調整）
        camera.new_cam_mat = cv::getOptimalNewCameraMatrix( camera.cam_mat,
                                                        camera.dist_coefs,
                                                        src.size(), 1);
        break;
    }

    while(1){
        // 姿勢(rvec/tvec)が未確定（マーカー未検出）の間は描画用変換をスキップ。
        // 空行列を行列演算に渡すと cv::Exception(empty matrix) で落ちるため。
        if(camera.im_ori.empty()||camera.rvec.empty()||camera.tvec.empty()){continue;}
        Mat src=camera.im_ori.clone();

        Mat hsv;
        cvtColor(src, hsv, COLOR_BGR2HSV);
        Mat proc;
        colorRange(hsv,proc,
                   camera.tgt.ch0_min, camera.tgt.ch0_max,
                   camera.tgt.ch1_min, camera.tgt.ch1_max,
                   camera.tgt.ch2_min, camera.tgt.ch2_max,
                   1);

        vector<Point2f> tgts=get_N_targetpoints(proc, camera.tgt.N);

        // ターゲット色が未検出だと tgts が空になる。空の点列を undistortPoints に渡すと
        // 内部の行列転置で cv::Exception(empty matrix) になって落ちるので、その場合はスキップ。
        if(tgts.empty()){ continue; }


        // 歪み補正後のピクセル→正規化座標
        // 共有Matはローカルにスナップショット(ヘッダを固定)し、3x3有効時のみ使う。
        // 別スレッドとの競合で一時的に空/非3x3になり undistortPoints が terminate する事故を防ぐ。
        cv::Mat K1 = camera.new_cam_mat;
        if(K1.empty() || K1.rows!=3 || K1.cols!=3){ continue; }
        std::vector<cv::Point2f> normPoints;
        try{
            cv::undistortPoints(tgts, normPoints, K1, cv::noArray());
        }catch(const cv::Exception&){ continue; }

        // 正規化画像座標からカメラ座標へ（任意の depth）
        float depth = 1000.0f; // 視錐台の長さ（チェスボード単位と合わせて調整）
        std::vector<cv::Point3f> camPoints;
        for (const auto& p : normPoints)
            camPoints.emplace_back(p.x * depth, p.y * depth, depth);



        // カメラ座標 → ワールド座標へ変換
        cv::Mat R;
        cv::Rodrigues(camera.rvec, R);
        std::vector<cv::Point3f> exportdata;
        for (const auto& pt : camPoints) {
            cv::Mat X_cam = (cv::Mat_<double>(3,1) << pt.x, pt.y, pt.z);
            cv::Mat worldPt = R.t() * (X_cam-camera.tvec);
            exportdata.push_back(cv::Point3f(worldPt));
        }

        trackdata_3d=exportdata;

    }
}

// 2個目のtgt: 画像中のArUcoマーカーから位置・姿勢・IDを出すスレッド。
// 流れ: detectMarkers で複数マーカーの4隅とIDを一括取得 → 各マーカーの4隅で
//        solvePnP(カメラ系姿勢) → カメラのワールド姿勢で世界系へ変換 → aruco_poses[id] に保存。
// ArUcoはQRより検出が桁違いに軽い(実測 約1.6ms vs 34ms)ため連続追跡に向く。
void aruco_tracking(){
    cv::aruco::Dictionary dict=cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    cv::aruco::DetectorParameters params;
    cv::aruco::ArucoDetector detector(dict, params);

    // マーカーローカル座標系での4隅(中心原点, z=0)。detectMarkers が返す順(TL,TR,BR,BL)に
    // 合わせ、y上向きで定義する。CGの向きが裏返る/90°ずれる場合はこの順序や符号を調整する。
    float h=(float)aruco_marker_size/2.0f;
    std::vector<cv::Point3f> markerObjectPoints={
        {-h,  h, 0.0f},  // top-left
        { h,  h, 0.0f},  // top-right
        { h, -h, 0.0f},  // bottom-right
        {-h, -h, 0.0f}   // bottom-left
    };

    while(1){
        // 間引き: ArUco検出は約1.6msと軽く、無制限に回すと共有データ(camera.im_ori,
        // rvec/tvec)への無同期アクセス頻度が跳ね上がり、cap/calib/tgt と競合して
        // 4点PnP・色追跡が崩れる。約30Hz(1/30秒)に抑えて衝突頻度とCPU占有を下げる。
        // ※これは対症療法で競合自体は残る。完全解決には共有Matの同期/スナップショットが要る。
        std::this_thread::sleep_for(std::chrono::milliseconds(33));

        // calib() がカメラのワールド姿勢(rvec/tvec)を確定するまで待つ。マーカー姿勢はまず
        // カメラ系で求め、その後カメラのワールド姿勢で世界系へ移すため両方が必要。
        if(camera.im_ori.empty()||camera.rvec.empty()||camera.tvec.empty()){
            for(int id=0;id<ARUCO_MAX_ID;id++) aruco_poses[id].found=false;
            continue;
        }
        Mat src=camera.im_ori.clone();

        // 複数マーカーを一括検出。corners[k] は各マーカーの4隅(TL,TR,BR,BL)、ids[k] はその整数ID。
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners, rejected;
        try{ detector.detectMarkers(src, corners, ids, rejected); }
        catch(const cv::Exception&){ ids.clear(); corners.clear(); }

        bool seen[ARUCO_MAX_ID]={false};
        if(!ids.empty()){
            // camera.rvec/tvec : ワールド→カメラ  (X_cam = R_wc*X_world + t_wc)
            cv::Mat R_wc; cv::Rodrigues(camera.rvec, R_wc);

            for(size_t k=0;k<ids.size();k++){
                int id=ids[k];                       // ArUcoはIDを整数で直接返す(復号不要)
                if(id<0||id>=ARUCO_MAX_ID) continue;

                // マーカーの姿勢をカメラ座標系で推定 (rvec_m/tvec_m: マーカーローカル→カメラ)
                cv::Mat rvec_m, tvec_m;
                if(!cv::solvePnP(markerObjectPoints, corners[k], camera.cam_mat, camera.dist_coefs,
                                 rvec_m, tvec_m, false, cv::SOLVEPNP_ITERATIVE)) continue;

                // カメラ系→ワールド系へ。
                //   rvec_m/tvec_m : マーカーローカル→カメラ (X_cam = R_cm*X_m + t_cm)
                // ∴ マーカー中心(X_m=0)のワールド位置 = R_wc^T (t_cm - t_wc)
                //    マーカーのワールド回転           = R_wc^T * R_cm
                cv::Mat R_cm; cv::Rodrigues(rvec_m, R_cm);
                cv::Mat R_wm=R_wc.t()*R_cm;
                cv::Mat pos =R_wc.t()*(tvec_m-camera.tvec);
                cv::Vec3d aa; cv::Rodrigues(R_wm, aa);
                double ang=cv::norm(aa);

                aruco_poses[id].pos=cv::Point3f((float)pos.at<double>(0),
                                                (float)pos.at<double>(1),
                                                (float)pos.at<double>(2));
                if(ang>1e-9){ aruco_poses[id].axis=aa/ang; aruco_poses[id].angle_deg=ang*180.0/CV_PI; }
                else        { aruco_poses[id].axis=cv::Vec3d(0,0,1); aruco_poses[id].angle_deg=0.0; }
                aruco_poses[id].found=true;
                aruco_poses[id].miss=0;     // 検出成功 → 見失いカウントをリセット
                seen[id]=true;
            }
        }
        // このフレームで見えなかったIDは、ARUCO_MISS_LIMIT回連続で外れて初めてCGを消す。
        // それまでは直近の姿勢を保持し、取りこぼしフレームでの点滅を防ぐ。
        for(int id=0;id<ARUCO_MAX_ID;id++){
            if(seen[id]) continue;
            if(++aruco_poses[id].miss>=ARUCO_MISS_LIMIT) aruco_poses[id].found=false;
        }
    }
}

void calib(){
   vector<Point3f> objectPoints = {
       Point3f(-4*lego, -4*lego, 0.0f),
       Point3f(4*lego, -4*lego, 0.0f),
       Point3f(4*lego, 4*lego, 0.0f),
       Point3f(-4*lego, 4*lego, 0.0f)
   };
    double unit=150.0;
    // vector<Point3f> objectPoints = {
    //     Point3f(0,0,0),
    //     Point3f(150.0,0,0),
    //     Point3f(0,100.0,0),
    //     Point3f(150.0,100.0,0)
    // };


    string yml="./output/cam_intrinsic_prameters_test.yml";


    cv::FileStorage fs(yml, cv::FileStorage::READ);
    fs["cameraMatrix"] >> camera.cam_mat;
    fs["distCoeffs"] >> camera.dist_coefs;
    fs.release();
    while(1){
        if(camera.im_ori.empty()){continue;}
        Mat src=camera.im_ori.clone();
        // 新しいカメラ行列（必要に応じて画像サイズやスケーリング係数を調整）
        camera.new_cam_mat = cv::getOptimalNewCameraMatrix(camera.cam_mat,
                                                        camera.dist_coefs,
                                                        camera.capsize, 1);
        break;
    }

    while(1){
        if(camera.im_ori.empty()){continue;}
        Mat src=camera.im_ori.clone();

        Mat hsv;
        cvtColor(src, hsv, COLOR_BGR2HSV);
        Mat proc;
        colorRange(hsv,proc,
                   camera.ori.ch0_min, camera.ori.ch0_max,
                   camera.ori.ch1_min, camera.ori.ch1_max,
                   camera.ori.ch2_min, camera.ori.ch2_max,
                   1);

        vector<Point2f> origin=get_N_targetpoints(proc, 1);

        colorRange(hsv,proc,
                   camera.oth.ch0_min, camera.oth.ch0_max,
                   camera.oth.ch1_min, camera.oth.ch1_max,
                   camera.oth.ch2_min, camera.oth.ch2_max,
                   3);

        vector<Point2f> others=get_N_targetpoints(proc, 3);

        vector<Point2f> merged;
        merged.reserve(origin.size() + others.size());
        merged.insert(merged.end(), origin.begin(), origin.end());
        merged.insert(merged.end(), others.begin(), others.end());

        // PnP には ori×1 + oth×3 の計4点が必須。検出が4点に満たないフレームでは
        // matchCorrespondingPoints が CV_Assert(==4) で例外を投げてクラッシュし、
        // そもそも姿勢推定もできないため、4点揃わないフレームはスキップする。
        if (merged.size() != 4) continue;

        if(hasDuplicatePoints(merged))continue;


        auto [matched2D, matched3D] = matchCorrespondingPoints(merged, objectPoints);



        impoint2d=matched2D;
        impoint3d=matched3D;
        for(int i=0;i<matched3D.size();i++){
            impoint3d[i].x=matched3D[i].y;
            impoint3d[i].y=matched3D[i].x;
        }

        bool success = cv::solvePnP(impoint3d,
                                    impoint2d,
                                    camera.cam_mat,
                                    camera.dist_coefs,
                                    camera.rvec,
                                    camera.tvec,
                                    false,
                                    cv::SOLVEPNP_ITERATIVE);

        if (!success) {
            std::cerr << "solvePnP failed!" << std::endl;
            continue;
        }

        std::vector<cv::Point2f> imgCorners = {
            {0, 0},
            {(float)camera.capsize.width, 0},
            {(float)camera.capsize.width, (float)camera.capsize.height},
            {0, (float)camera.capsize.height}
        };

        // 歪み補正後のピクセル→正規化座標
        // 共有Matはローカルにスナップショット(ヘッダを固定)し、3x3有効時のみ使う。
        cv::Mat K0 = camera.cam_mat, D0 = camera.dist_coefs;
        if(K0.empty() || K0.rows!=3 || K0.cols!=3){ continue; }
        std::vector<cv::Point2f> normPoints;
        try{
            cv::undistortPoints(imgCorners, normPoints, K0, D0);
        }catch(const cv::Exception&){ continue; }

        // 正規化画像座標からカメラ座標へ（任意の depth）
        // 底面クアッドは、setupOpenGLProjectionAndView の glFrustum と同じ「矩形」で作る。
        //   left=normPoints[0].x(TL), right=normPoints[1].x(TR),
        //   top =normPoints[0].y(TL), bottom=normPoints[2].y(BR)
        // 4隅を独立に使うと歪み/principal point ずれで非矩形になり、矩形フラスタムと
        // 一致せず底面がビューポート端に揃わない。矩形に揃えることで4隅とも一致する。
        float depth = 100.0f;
        float L = normPoints[0].x, Rr = normPoints[1].x;
        float T = normPoints[0].y, B  = normPoints[2].y;
        std::vector<cv::Point3f> camPoints = {
            {L  * depth, T * depth, depth},   // TL (imgCorners[0])
            {Rr * depth, T * depth, depth},   // TR (imgCorners[1])
            {Rr * depth, B * depth, depth},   // BR (imgCorners[2])
            {L  * depth, B * depth, depth}    // BL (imgCorners[3])
        };



        // カメラ座標 → ワールド座標へ変換
        cv::Mat R;
        cv::Rodrigues(camera.rvec, R);
        std::vector<cv::Point3f> exportdata;
        for (const auto& pt : camPoints) {
            cv::Mat X_cam = (cv::Mat_<double>(3,1) << pt.x, pt.y, pt.z);
            cv::Mat worldPt = R.t() * (X_cam-camera.tvec);
            exportdata.push_back(cv::Point3f(worldPt));
        }

        exportdata.insert(exportdata.end(),
                          camPoints.begin(),
                          camPoints.end());


        cv::Mat cameraPosition = -R.t() * camera.tvec;
        cam=Point3f(cameraPosition);
        transport_3d=exportdata;
        vector<Mat> exportcam={camera.cam_mat,camera.dist_coefs,camera.rvec,camera.tvec};
        cameraData=exportcam;
    }
}





}  // anonymous namespace

int main7(int argc, char *argv[])
{
    std::cout << "Current path: " << std::filesystem::current_path() << std::endl;

    std::thread thread1(cap);
    std::thread thread2(calib);
    std::thread thread3(tgt_tracking);
    std::thread thread4(aruco_tracking);   // 2個目のtgt: ArUcoマーカーの位置・姿勢

    app_gl.setDisplayFunction(myDisplay_7);  // ユーザーの描画関数を登録
    app_gl.setAddDisplayFunction(additionalDisplay_7);  // ユーザーの描画関数をデフォルトに追加
    app_gl.setWindowSize(1500, 800);  // ユーザーの描画関数を登録

    app_gl.init(argc, argv);

    // 注: POVfrom/POVto をここで設定しても idle()->update_visualObj() が毎フレーム
    // POVfrom = casts[Camera] で上書きするため無効。初期視点は OpenGL_world.hpp の
    // 'obj'(casts[Camera] の初期値) で決まる。

    loadOBJ(models[0],"../common_data/CG_objects/lego.obj");
    glmScale(models[0],40);
    glmTranslateModel(models[0], 0.0f, 0.0f, 40.0/2.0);

    loadOBJ(models[1],"../common_data/CG_objects/wall.obj");
    glmScale(models[1],40.1);
    glmTranslateModel(models[1], 0.0f, 0.0f, 40.1/2.0);
    Vec3 size=getBoundingBoxSize(models[1]);

    // LoadGLTextures("../common_data/test_image.png", texture_7[1]);

    GLfloat light_pos[] = {0,0,100.0f,0};
    GLfloat light_amb[] = { 0.5, 0.5, 0.5, 1.0};
    GLfloat light_dif[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_spec[]= { 0.5, 0.5, 0.5, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_spec);

    app_gl.run();



    return(0);
}
