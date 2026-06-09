//
//  opengl_world.cpp
//  opengl_class_study
//
//  Created by nozomi nishiumi on 2025/05/06.
//
#define GL_SILENCE_DEPRECATION

#include "OpenGL_world.hpp"
#include <string>
#include <algorithm>


// バウンディングボックスのサイズを返す関数
Vec3 getBoundingBoxSize(GLMmodel* model) {

    if (!model || !model->numvertices || !model->vertices) {
        return {0.0, 0.0, 0.0}; // エラーハンドリング
    }

    double minX = model->vertices[3], maxX = model->vertices[3];
    double minY = model->vertices[4], maxY = model->vertices[4];
    double minZ = model->vertices[5], maxZ = model->vertices[5];

    for (unsigned int i = 1; i < model->numvertices; ++i) {
        double x = model->vertices[3 * i + 0];
        double y = model->vertices[3 * i + 1];
        double z = model->vertices[3 * i + 2];

        if (x < minX) minX = x;
        if (x > maxX) maxX = x;

        if (y < minY) minY = y;
        if (y > maxY) maxY = y;

        if (z < minZ) minZ = z;
        if (z > maxZ) maxZ = z;
    }

    Vec3 size;
    size.x = maxX - minX;
    size.y = maxY - minY;
    size.z = maxZ - minZ;
    return size;
}


std::filesystem::path getThisFileDirectory() {
    std::filesystem::path fullPath(__FILE__);
    return fullPath.parent_path();
}

void MyGLApp::pramInit(){
    casts[Camera][X]=1000;
    casts[Camera][Y]=0;
    casts[Camera][Z]=50;
    casts[Camera][hLOS]=0;
    casts[Camera][vLOS]=0;
    casts[Camera][hHEAD]=pi*4.0/4.0;
    casts[Camera][vHEAD]=0;
    casts[Camera][velocity]=0;

    rot[0]=casts[Camera][hHEAD];
    rot[1]=casts[Camera][vHEAD];
}

void MyGLApp::steering2(){
    double a=0.1;
    if(steerL){steer[0]+=a;}else{steer[0]-=a;}
    if(steerR){steer[1]+=a;}else{steer[1]-=a;}
    if(steerD){steer[2]+=a;}else{steer[2]-=a;}
    if(steerU){steer[3]+=a;}else{steer[3]-=a;}

    for(int i=0;i<4;i++){
        if(steer[i]<0.0){steer[i]=0.0;}
        if(steer[i]>1.0){steer[i]=1.0;}
    }
}



void MyGLApp::key_down(int key, int x, int y){
    switch(key){
        case GLUT_KEY_UP:
            steerU=true;
            break;
        case GLUT_KEY_DOWN:
            steerD=true;
            break;
        case GLUT_KEY_RIGHT:
            steerR=true;
            break;
        case GLUT_KEY_LEFT:
            steerL=true;
            break;
    }
}
void MyGLApp::key_up(int key, int x, int y){
    switch(key){
        case GLUT_KEY_UP:
            steerU=false;
            break;
        case GLUT_KEY_DOWN:
            steerD=false;
            break;
        case GLUT_KEY_RIGHT:
            steerR=false;
            break;
        case GLUT_KEY_LEFT:
            steerL=false;
            break;
    }
}

void MyGLApp::key_down_nml(unsigned char key, int x, int y){
    switch(key){
        case 'z':
            zzz=true;
            break;
        case 'x':
            xxx=true;
            break;
        case 'a':
            aaa=true;
            break;
        case 's':
            sss=true;
            break;
        case 'b':
            cout<<"xyz: "<<casts[Camera][X]<<","<<casts[Camera][Y]<<","<<casts[Camera][Z]<<endl;

    }
}



void MyGLApp::key_up_nml(unsigned char key, int x, int y){
    switch(key){
        case 'z':
            zzz=false;
            break;
        case 'x':
            xxx=false;
            break;
        case 'a':
            aaa=false;
            break;
        case 's':
            sss=false;
            break;
    }
}

void MyGLApp::normalize(){
    for(int i=0; i<4; i++){
        if (rot[i] > pi) {
            rot[i] -= 2 * pi;
        }
        if (rot[i] < -pi) {
            rot[i] += 2 * pi;
        }
    }
}

void MyGLApp::param(){
    if(zzz){
        vangle+=1;
        if(vangle>179){vangle = 179;}
    }
    if(xxx){
        vangle-=1;
        if(vangle<1){vangle = 1;}
    }

    if(aaa){
        casts[Camera][velocity]+=3;
        if(casts[Camera][velocity]>Vmax){casts[Camera][velocity] = Vmax;
        }
    }

    if(sss){
        casts[Camera][velocity]-=3;
        if(casts[Camera][velocity]<0){casts[Camera][velocity] = 0;}
    }
}

void MyGLApp::ODE_evolve_2(const double t,
                  const double dt,
                  vector<vector<double>> &x,
                  void(*ode)(
                             vector<vector<double>> &,
                             vector<vector<double>> &)
                  ) {
    vector<vector<double>> x_next((int)x.size(),vector<double> ((int)x[0].size()));
    // Eular法: 実際のコードで使ってはならない!!
    vector<vector<double>> dx((int)x.size(),vector<double> ((int)x[0].size()));

    ode(dx,x);
    //    delta_casts=dx;
    for (int i = 0; i < x.size(); i++) {
        for(int j=0;j<x[0].size();j++){
            x_next[i][j] = x[i][j] + dx[i][j] * dt;
        }
    }
    x=x_next;
    return;
}

void MyGLApp::pilotControl_P_2(vector<double> &dx, vector<double> &x, double steering[4]){

    dx[hHEAD]=pow(steer[0],2)-pow(steer[1],2);
    dx[vHEAD]=pow(steer[2],2)-pow(steer[3],2);

    int direction_h, direction_v;
    direction_h = (dx[hHEAD] > 0) - (dx[hHEAD] < 0);
    direction_v = (dx[vHEAD] > 0) - (dx[vHEAD] < 0);

    if (sqrt(dx[hHEAD] * dx[hHEAD]) > turnlimit){
        dx[hHEAD] = (double)direction_h*turnlimit;
    }
    if (sqrt(dx[vHEAD] * dx[vHEAD]) > turnlimit){
        dx[vHEAD] = (double)direction_v*turnlimit;
    }


    dx[X] = x[velocity]*cos(x[hHEAD])*cos(x[vHEAD]);
    dx[Y] = x[velocity]*sin(x[hHEAD])*cos(x[vHEAD]);
    dx[Z] = x[velocity]*sin(x[vHEAD]);
}

void MyGLApp::navigation_guidance_2(vector<vector<double>> &dx,
                           vector<vector<double>> &x) {

    pilotControl_P_2(dx[Camera], x[Camera], steer);

    return;
}

void MyGLApp::navigation_guidance_2_wrapper(std::vector<std::vector<double>> &dx,
                                          std::vector<std::vector<double>> &x) {
    MyGLApp::getInstance().navigation_guidance_2(dx, x);
}



void MyGLApp::update_visualObj(){
    body.erase(body.begin(),body.end());
    head.erase(head.begin(),head.end());

    body.push_back(casts[Camera]);
    head.push_back(Camera_head);


    for(int i=0;i<3;i++){
        Head[i]=head[tgtcast][i];

        POVfrom[i]=body[tgtcast][i];
        if(tgtcast==tgtlock){
            POVto[i]=head[tgtcast][i];
        }else{
            POVto[i]=body[tgtlock][i];
        }
    }
}





MyGLApp& MyGLApp::getInstance() {
    static MyGLApp instance;
    return instance;
}


MyGLApp::MyGLApp() {
    userDisplayFunc = MyGLApp::defaultDisplayFunc();
    userAddDisplayFunc = MyGLApp::defaultAddDisplayFunc();

}

std::function<void()> MyGLApp::defaultAddDisplayFunc() {
    return [this]() {
    };
}

std::function<void()> MyGLApp::defaultDisplayFunc() {
    return [this]() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //バッファの消去

        glClearColor(0.0, 0.0, 0.0, 0.0);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();//行列の初期化
        double aspectratio=(double)WindowWidth / (double)WindowHeight;
        double verticalangle=2*atan(tan(0.5*vangle/180.0*pi)/aspectratio)/pi*180.0;
        //視点の設定------------------------------
        gluPerspective(verticalangle,aspectratio, 0.01, 1000000000.0);

        int flip=-1;
        if(fmod(fabs(casts[Camera][vHEAD]),2.0*pi)>0.5*pi&&fmod(fabs(casts[Camera][vHEAD]),2.0*pi)<1.5*pi){
            flip=-1;
        }else{flip=1;}




        gluLookAt(POVfrom[0],POVfrom[1],POVfrom[2],
                  POVto[0],POVto[1],POVto[2],
                  0, 0, flip);



        //----------------------------------------

        //モデルビュー変換行列の設定--------------------------
        //行列モードの設定
        // (GL_PROJECTION : 透視変換行列の設定、GL_MODELVIEW：モデルビュー変換行列)
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();//行列の初期化
        glViewport(0, 0, WindowWidth, WindowHeight);
        //----------------------------------------------

        glDisable(GL_LIGHTING);


        drawGround(vector<double> {POVfrom[0],POVfrom[1],POVfrom[2]},
                   100,0.4,0.4,0.4);


        //陰影ON-----------------------------
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);//光源0を利用






    };
}

void MyGLApp::idle() {
    normalize();
    steering2();
    param();

    const int elapsed_time_now = glutGet(GLUT_ELAPSED_TIME);

    ODE_evolve_2(elapsed_time_now,
                 1.0/60.0,
                 casts,
                 MyGLApp::navigation_guidance_2_wrapper);

    Camera_head[0]=casts[Camera][X]+cos(casts[Camera][hHEAD])*cos(casts[Camera][vHEAD]);
    Camera_head[1]=casts[Camera][Y]+sin(casts[Camera][hHEAD])*cos(casts[Camera][vHEAD]);
    Camera_head[2]=casts[Camera][Z]+sin(casts[Camera][vHEAD]);

    update_visualObj();


    int delta_time=elapsed_time_now-elapsed_time;
    int additional_time=1000.0/60.0-delta_time;
    elapsed_time = elapsed_time_now;

    std::this_thread::sleep_for(std::chrono::milliseconds(additional_time));


    glutSetWindow(MainWinID[0]);
    glutPostRedisplay();
}





void MyGLApp::init(int argc, char** argv) {
    glutInit(&argc, argv);
    WindowWidth = 2560/2;
    WindowHeight = 1664/2;
    MainWinID[2];

    glutInitWindowPosition(WindowPositionX, WindowPositionY);//ウィンドウの位置の指定
    glutInitWindowSize(WindowWidth, WindowHeight); //ウィンドウサイズの指定
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL);//ディスプレイモードの指定
    MainWinID[0]=glutCreateWindow("user interface");
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA , GL_DST_ALPHA);

    update_visualObj();


    glutDisplayFunc(displayCallback);
    glutIdleFunc(idleCallback);
    glutKeyboardFunc(keyboardCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(keyboardSpecCallback);
    glutSpecialUpFunc(keyboardSpecUpCallback);

    GLfloat light_amb[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat light_dif[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_spec[]= { 0.1f, 0.1f, 0.1f, 0.1f };
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_spec);



}

void MyGLApp::run() {
    glutMainLoop();
}

void MyGLApp::setDisplayFunction(const std::function<void()>& func) {
    userDisplayFunc = func;
}

void MyGLApp::setAddDisplayFunction(const std::function<void()>& func) {
    userAddDisplayFunc = func;
}
void MyGLApp::setWindowSize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
}

void MyGLApp::displayCallback() {
    getInstance().display();
}

void MyGLApp::idleCallback() {
    getInstance().idle();
}

void MyGLApp::keyboardCallback(unsigned char key, int x, int y) {
    getInstance().key_down_nml(key, x, y);
}

void MyGLApp::keyboardUpCallback(unsigned char key, int x, int y) {
    getInstance().key_up_nml(key, x, y);
}

void MyGLApp::keyboardSpecCallback(int key, int x, int y) {
    getInstance().key_down(key, x, y);
}
void MyGLApp::keyboardSpecUpCallback(int key, int x, int y) {
    getInstance().key_up(key, x, y);
}

void MyGLApp::display() {

    // 登録された描画処理を呼び出す
    if (userDisplayFunc) {
        userDisplayFunc();
    }
    if (userAddDisplayFunc) {
        userAddDisplayFunc();
    }

    glutSwapBuffers();

}


void MyGLApp::keyboard(unsigned char key, int x, int y) {
    if (key == 27) {
        exit(0);
    }
    std::cout << "Key pressed: " << key << std::endl;
}


bool CreateFBO(FBO& outFBO, int width, int height) {
    outFBO.width = width;
    outFBO.height = height;

    glGenFramebuffers(1, &outFBO.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, outFBO.fbo);

    // カラーテクスチャ
    glGenTextures(1, &outFBO.colorTex);
    glBindTexture(GL_TEXTURE_2D, outFBO.colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, outFBO.colorTex, 0);

    // 深度レンダーバッファ
    glGenRenderbuffers(1, &outFBO.depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, outFBO.depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, outFBO.depthRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "FBO creation failed!\n");
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, outFBO.fbo);
    return true;
}



void EndFBO() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DrawFBO_Additive(const FBO& fbo) {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);  // NDC

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fbo.colorTex);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glPopMatrix(); // modelview
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void DeleteFBO(FBO& fbo) {
    if (fbo.fbo) glDeleteFramebuffers(1, &fbo.fbo);
    if (fbo.colorTex) glDeleteTextures(1, &fbo.colorTex);
    if (fbo.depthRbo) glDeleteRenderbuffers(1, &fbo.depthRbo);
    fbo = FBO(); // zero clear
}





void drawGround(const vector<double> eye,int order,double R, double G, double B) {
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA , GL_DST_ALPHA);
    glEnable(GL_BLEND);

    const double ground_max_x = 60000;
    const double ground_max_y = 60000;

    int basepoint[2]={0,0};

    glColor4d(R,G,B,0.5);
    glBegin(GL_LINES);
    for (double ly = -ground_max_y+basepoint[1]; ly <= ground_max_y+basepoint[1]; ly += order) {
        glVertex3d(-ground_max_x+basepoint[0], ly, 0);
        glVertex3d(ground_max_x+basepoint[0], ly, 0);
    }
    for (double lx = -ground_max_x+basepoint[0]; lx <= ground_max_x+basepoint[0]; lx += order) {
        glVertex3d(lx, ground_max_y+basepoint[1], 0);
        glVertex3d(lx, -ground_max_y+basepoint[1], 0);
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);



}

void glmTransform(GLMmodel* model, GLfloat matrix[4][4]) {
    if (!model || !model->vertices)
        return;  // モデルまたは頂点が無効な場合は処理しない

    GLuint i;
    GLfloat v[3];

    for (i = 1; i <= model->numvertices; i++) {
        v[0] = model->vertices[3 * i + 0];
        v[1] = model->vertices[3 * i + 1];
        v[2] = model->vertices[3 * i + 2];

        // 行列を適用
        model->vertices[3 * i + 0] =
            matrix[0][0] * v[0] + matrix[0][1] * v[1] + matrix[0][2] * v[2] + matrix[0][3];
        model->vertices[3 * i + 1] =
            matrix[1][0] * v[0] + matrix[1][1] * v[1] + matrix[1][2] * v[2] + matrix[1][3];
        model->vertices[3 * i + 2] =
            matrix[2][0] * v[0] + matrix[2][1] * v[1] + matrix[2][2] * v[2] + matrix[2][3];
    }
}

void glmRotateModel(GLMmodel* model, float angleDeg, float x, float y, float z) {
    float rad = angleDeg * M_PI / 180.0f;

    // 回転軸を正規化
    float len = sqrtf(x * x + y * y + z * z);
    if (len == 0.0f) return;  // 無効な軸
    x /= len;
    y /= len;
    z /= len;

    float c = cosf(rad);
    float s = sinf(rad);
    float t = 1.0f - c;

    // Rodrigues の回転行列
    GLfloat m[4][4] = {
        { t*x*x + c,     t*x*y - s*z, t*x*z + s*y, 0 },
        { t*x*y + s*z,   t*y*y + c,   t*y*z - s*x, 0 },
        { t*x*z - s*y,   t*y*z + s*x, t*z*z + c,   0 },
        { 0,             0,           0,           1 }
    };

    glmTransform(model, m);
}

void glmTranslateModel(GLMmodel* model, float dx, float dy, float dz) {
    GLfloat m[4][4] = {
        { 1.0f, 0.0f, 0.0f, dx },
        { 0.0f, 1.0f, 0.0f, dy },
        { 0.0f, 0.0f, 1.0f, dz },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    glmTransform(model, m);
}


void glmBoundingBox(GLMmodel* model, float min[3], float max[3]) {
    if (!model || !model->vertices) return;

    min[0] = min[1] = min[2] =  0;
    max[0] = max[1] = max[2] = 0;

    for (GLuint i = 1; i <= model->numvertices; i++) {
        float x = model->vertices[3 * i + 0];
        float y = model->vertices[3 * i + 1];
        float z = model->vertices[3 * i + 2];

        if (x < min[0]) min[0] = x;
        if (y < min[1]) min[1] = y;
        if (z < min[2]) min[2] = z;

        if (x > max[0]) max[0] = x;
        if (y > max[1]) max[1] = y;
        if (z > max[2]) max[2] = z;
    }
}

void makeShadowMatrix(GLfloat shadowMat[16], const GLfloat light[4], const GLfloat plane[4]) {
    float A = plane[0], B = plane[1], C = plane[2], D = plane[3];
    float Lx = light[0], Ly = light[1], Lz = light[2], Lw = light[3];

    float dot = A*Lx + B*Ly + C*Lz + D*Lw;

    shadowMat[0]  = dot - Lx*A;
    shadowMat[4]  =    - Lx*B;
    shadowMat[8]  =    - Lx*C;
    shadowMat[12] =    - Lx*D;

    shadowMat[1]  =    - Ly*A;
    shadowMat[5]  = dot - Ly*B;
    shadowMat[9]  =    - Ly*C;
    shadowMat[13] =    - Ly*D;

    shadowMat[2]  =    - Lz*A;
    shadowMat[6]  =    - Lz*B;
    shadowMat[10] = dot - Lz*C;
    shadowMat[14] =    - Lz*D;

    shadowMat[3]  =    - Lw*A;
    shadowMat[7]  =    - Lw*B;
    shadowMat[11] =    - Lw*C;
    shadowMat[15] = dot - Lw*D;
}

void drawSoftShadow(const GLfloat light[4], const GLfloat plane[4], void (*drawObject)()) {
    GLfloat shadowMat[16];
    makeShadowMatrix(shadowMat, light, plane);

    // ブレンド設定
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < 4; ++i) {
        float scale = 1.0f + i * 0.05f;       // 徐々に拡大
        float alpha = 0.5f / (i + 1);        // 外ほど薄く

        glPushMatrix();
            glMultMatrixf(shadowMat);         // 影行列で投影
            glScalef(scale, 1.0f, scale);     // xz平面方向だけスケール
            glColor4f(0.0f, 0.0f, 0.0f, alpha);
            drawObject();                     // 影として描画
        glPopMatrix();
    }


//    glDisable(GL_BLEND);
}


// ----------------------------------------------------------------
// グローバル定義
// ----------------------------------------------------------------
MyGLApp& app_gl = MyGLApp::getInstance();
int frame_gl = 0;

void viewsetting() {
    MyGLApp& app = MyGLApp::getInstance();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(app.vangle, 1500.0/800.0, 0.01, 1000000000.0);
    gluLookAt(app.POVfrom[0], app.POVfrom[1], app.POVfrom[2], 0, 0, 0, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, 1500, 800);
}

void loadOBJ(GLMmodel*& model, const std::string& filepath) {
    model = glmReadOBJ((char*)filepath.c_str());
    if (!model) {
        fprintf(stderr, "OBJファイルの読み込みに失敗しました: %s\n", filepath.c_str());
        exit(1);
    }
    glmFacetNormals(model);
    glmVertexNormals(model, 90.0f);
    glmRotateModel(model, 90.0f, 0.0f, 0.0f, 1.0f);
    glmUnitize(model);
    Vec3 size = getBoundingBoxSize(model);
    double maxsize = std::max({size.x, size.y, size.z});
    glmScale(model, 1.0/maxsize);
}
