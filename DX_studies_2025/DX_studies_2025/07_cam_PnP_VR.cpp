#define GL_SILENCE_DEPRECATION
#define lego 8.0
#include "OpenCV_functions.hpp"
#include "OpenGL_world.hpp"
#include "CV_GL_combination.hpp"
#include <thread>
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





void viewsetting2(){
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //バッファの消去

    setupOpenGLProjectionAndView(cameraData[0], cameraData[1], Size(camera.im_ori.cols,camera.im_ori.rows), camera.rvec, camera.tvec);
    glViewport(0, 0, (double)camera.im_ori.cols/(double)camera.im_ori.rows*800.0, 800);
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
    if (models[1]&&trackdata_3d.size()>0&&
        cam.x!=0&&cam.y!=0&&cam.z!=0&&
        trackdata_3d.size()>0){
        vector<Point3f> tgt=intersect(cam,trackdata_3d,0);
        glPushMatrix();
        glTranslated(tgt[0].x, tgt[0].y, tgt[0].z);
        glmDraw(models[1], GLM_SMOOTH | GLM_COLOR);
        glPopMatrix();
}
}

void draw_GhostCGmodels(){
    if (models[1]&&trackdata_3d.size()>0&&
        cam.x!=0&&cam.y!=0&&cam.z!=0&&
        trackdata_3d.size()>0){
        vector<Point3f> tgt=intersect(cam,trackdata_3d,0);

        glDisable(GL_LIGHTING);
        glPushMatrix();
        glTranslated(tgt[0].x, tgt[0].y, tgt[0].z);
        glColor4d(0, 0.3, 0, 0.05);
        glmDraw(models[1], GLM_SMOOTH);
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

    


    draw_CGmodels();

}


void FBO_layer(){
    glClear(GL_DEPTH_BUFFER_BIT);
//    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    FBO fboObj2;
    double aspect;
    if (camera.capsize.width > 0 && camera.capsize.height > 0) {
        aspect = (double)camera.capsize.width / (double)camera.capsize.height;
    } else {
        aspect = 16.0 / 9.0;
    }
    CreateFBO(fboObj2, aspect * 800.0, 800);
    FBO_contents();
    EndFBO();
    DrawFBO_Additive(fboObj2);
    DeleteFBO(fboObj2);
}

void occulusion_viewer(){
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // 色出力オフ
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_TRUE);                                // 深度は書き込む
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glPushMatrix();
    glTranslated(0, -100, 0);
    glmDraw(models[3], GLM_SMOOTH| GLM_COLOR);
    glPopMatrix();
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // 色出力オン
    glDepthFunc(GL_GREATER); // aより奥の部分だけ通過

    draw_CGmodels();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    glTranslated(0, -100, 0);
    glmDraw(models[3], GLM_SMOOTH| GLM_COLOR);
    glPopMatrix();
    glDisable(GL_DEPTH_TEST);
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

    capture.set(CAP_PROP_FRAME_WIDTH, 1920);
    capture.set(CAP_PROP_FRAME_HEIGHT, 1080);


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

        vector<Point2f> tgts=get_N_targetpoints(proc, 1);

        // ターゲット色が未検出だと tgts が空になる。空の点列を undistortPoints に渡すと
        // 内部の行列転置で cv::Exception(empty matrix) になって落ちるので、その場合はスキップ。
        if(tgts.empty()){ continue; }


        // 歪み補正後のピクセル→正規化座標
        std::vector<cv::Point2f> normPoints;
        cv::undistortPoints(tgts, normPoints, camera.new_cam_mat, cv::noArray());

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

void calib(){
//    vector<Point3f> objectPoints = {
//        Point3f(-5*lego, -5*lego, 0.0f),
//        Point3f(5*lego, -5*lego, 0.0f),
//        Point3f(5*lego, 5*lego, 0.0f),
//        Point3f(-5*lego, 5*lego, 0.0f)
//    };
    double unit=150.0;
    vector<Point3f> objectPoints = {
        Point3f(0,0,0),
        Point3f(150.0,0,0),
        Point3f(0,100.0,0),
        Point3f(150.0,100.0,0)
    };


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
        std::vector<cv::Point2f> normPoints;
        cv::undistortPoints(imgCorners, normPoints, camera.cam_mat, camera.dist_coefs);

        // 正規化画像座標からカメラ座標へ（任意の depth）
        float depth = 100.0f;
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

    app_gl.setDisplayFunction(myDisplay_7);  // ユーザーの描画関数を登録
    app_gl.setAddDisplayFunction(additionalDisplay_7);  // ユーザーの描画関数をデフォルトに追加
    app_gl.setWindowSize(1500, 800);  // ユーザーの描画関数を登録

    app_gl.POVfrom[0]=-300;
    app_gl.POVfrom[1]=-300;
    app_gl.POVfrom[2]=500;
    app_gl.POVto[0]=0;
    app_gl.POVto[1]=0;
    app_gl.POVto[2]=0;

    app_gl.init(argc, argv);

    loadOBJ(models[0],"../common_data/CG_objects/CH47.obj");
    glmScale(models[0],400);
    Vec3 size=getBoundingBoxSize(models[0]);
    glmTranslateModel(models[0], 0.0f, 0.0f, 89.3357544/2.0);

    loadOBJ(models[1],"../common_data/CG_objects/lego.obj");
    glmScale(models[1],40);
    glmTranslateModel(models[1], 0.0f, 0.0f, 40.0/2.0);

    loadOBJ(models[2],"../common_data/CG_objects/wall.obj");
    glmScale(models[2],40.1);
    glmTranslateModel(models[2], 0.0f, 0.0f, 40.1/2.0);
    size=getBoundingBoxSize(models[2]);

    loadOBJ(models[3],"../common_data/CG_objects/ripstick.obj");
    glmScale(models[3],70.0);
    glmTranslateModel(models[3], 0.0f, 0.0f, 40.1/2.0);
    size=getBoundingBoxSize(models[3]);

    // LoadGLTextures("../common_data/test_image.png", texture_7[1]);

    GLfloat light_pos[] = {0,0,100.0f,0};
    GLfloat light_amb[] = { 0.0, 0.0, 0.0, 1.0};
    GLfloat light_dif[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_spec[]= { 1.0, 1.0, 1.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_spec);

    app_gl.run();



    return(0);
}
