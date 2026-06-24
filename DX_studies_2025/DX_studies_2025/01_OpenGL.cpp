#define GL_SILENCE_DEPRECATION


#include "OpenGL_world.hpp"
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

MyGLApp& app = MyGLApp::getInstance();
double pi=3.14159265359;
int frame=0;
//vector<GLMmodel*>  models;
//vector<GLuint> model_lists;
GLMmodel* model = nullptr;



void lightSetting(){
    GLfloat light_pos[] = { 0.0f,1000.0f*(GLfloat)cos((GLfloat)frame/1000.0), 1000.0f*(GLfloat)sin((GLfloat)frame/1000.0), 1.0f };
    GLfloat light_amb[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat light_dif[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_spec[]= { 0.1f, 0.1f, 0.1f, 0.1f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_spec);

}


void view_setting(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //バッファの消去

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();//行列の初期化

    gluPerspective(app.vangle,1500.0/800.0, 0.01, 1000000000.0);
    gluLookAt(app.POVfrom[0],app.POVfrom[1],app.POVfrom[2],
              app.POVto[0],app.POVto[1],app.POVto[2],
              0, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();//行列の初期化
    glViewport(0, 0, 1500, 800);
}

void free4Students(){
    glDisable(GL_LIGHTING);
    drawGround(vector<double> {app.POVfrom[0],app.POVfrom[1],app.POVfrom[2]},
               100,0.4,0.4,0.4);


    glColor3d(1,1,1);
    glRasterPos3f(100,0,0);
     char* str="X";
           glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str);

    glRasterPos3f(0,100,0);
    str="Y";
           glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str);

    glRasterPos3f(0,0,0);
    str="O";
           glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str);


    glPointSize(5);
    glBegin(GL_POINTS);
    glVertex3d(100,0,0);
    glVertex3d(0,100,0);
    glVertex3d(0,0,0);
    glEnd();

    glColor3d(1, 0.7, 0.7);
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex3d(-200,60,1);
    glVertex3d(30, 30,1);
    glVertex3d(20,-50,1);
    glVertex3d(20,-20,1);
    glEnd();


    glColor3d(0.7, 0.7, 1);
    glPushMatrix();
    glTranslated(150, -50, 0);
    glRotated(45, 0, 0, 1);
    glLineWidth(3);
    glBegin(GL_LINE_LOOP);
    glVertex3d(-50,0,0);
    glVertex3d(50,0,0);
    glVertex3d(50,50,0);
    glVertex3d(0,50.0*sqrt(3),0);
    glEnd();
    glPopMatrix();


    glTranslated(-100, -100, 0);
    glColor3d( 0.7,1.0, 0.7);
    glLineWidth(3);
    glBegin(GL_POLYGON);
    glVertex3d(-50,0,0);
    glVertex3d(50,0,0);
    glVertex3d(0,50.0*sqrt(3),0);
    glEnd();



    float t = glutGet(GLUT_ELAPSED_TIME) / 5000.0f;



}

void draw_CGs(){
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    lightSetting();

    glPushMatrix();
    glTranslated(100, -150, 0);
    glutSolidCube(20);

    glTranslated(100, 0, 0);
    glutWireSphere(30,100,20);
    glPopMatrix();
    
    glPushMatrix();
    glTranslated(0.0f,1000.0f*(GLfloat)cos((GLfloat)frame/1000.0), 1000.0f*(GLfloat)sin((GLfloat)frame/1000.0));
    glColor3d(1,1,1);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glutSolidSphere(30,100,20);
    glPopMatrix();


    glColor3d(1, 0.7, 0.7);
    glTranslated(0, -500, 0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
     if (model) {
        glmDraw(model, GLM_SMOOTH | GLM_COLOR);
    }

    glTranslated(50, 0, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (model) {
        glmDraw(model, GLM_SMOOTH);
    }

    glDisable(GL_LIGHTING);
    glTranslated(50, 0, 0);
    glRotated(90, 0, 0, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (model) {
        glmDraw(model, GLM_SMOOTH);
    }

}

void myDisplay() {
    frame++;//フレームカウント
    view_setting();
    free4Students();
    draw_CGs();

}

void additionalDisplay() {

}

int main1(int argc, char** argv) {
    std::cout << "Current path: " << std::filesystem::current_path() << std::endl;

    
    model = glmReadOBJ("../common_data/CG_objects/lego.obj");

    if (!model) {
        fprintf(stderr, "OBJファイルの読み込みに失敗しました\n");
        exit(1);
    }

    glmFacetNormals(model);
    glmVertexNormals(model, 90.0);
    glmUnitize(model);
    glmScale(model,100.0);
    glmTranslateModel(model, 0.0f, 0.0f, 100.0/2.0);

    app.setDisplayFunction(myDisplay);  // ユーザーの描画関数を登録
    app.setAddDisplayFunction(additionalDisplay);  // ユーザーの描画関数をデフォルトに追加
    app.setWindowSize(1500, 800);  // ユーザーの描画関数を登録

    app.init(argc, argv);
    app.run();
    return 0;
}
