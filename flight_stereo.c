/*
 *
 * Thabang Ngazimbi
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#endif

#if defined (__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "readoff.h"
#include "readppm.h"
#include "glm.h"

/*****************************************************************/
/******************** MY FUNCTION DEFINITIONS ********************/
/*****************************************************************/

static void myInitGL(void);
static void myInitData(int *argc, char **argv);
static void myInitMenus (void);
static void myInitCheckers(void);
static void myDrawTeapot(GLdouble size);

/* my callback functions needed for GLUT event handling */
static void myHandleMenu(int value);
static void myDisplay(void);
static void myReshape(int width, int height);
static void myMotion(int curx, int cury);
static void myButton(int button, int state, int x, int y);
static void myVisibility(int state);
static void myKeyboard(unsigned char key, int x, int y);
static void myMenus (int value);

static void timerCallback (int value);
void myRollBall (GLfloat pDeltaX, GLfloat pDeltaY, GLfloat *pModelMatrix );

void drawmodel(void);
void drawscene(void);
void drawcircle(float radius);


void pos_nose_camera(void);
void vec_mat_mult( GLfloat M[], GLfloat vec[], GLfloat out_vec[]);

void output(int x, int y, char *string);

#define Radius 10

float normal[3] = {1.0,0.0,0.0};


GLMmodel* pmodel = NULL;
GLMmodel* smodel = NULL;

/***************************************************************/
/******************** MY GLOBAL CONSTANTS **********************/
/***************************************************************/

#define MY_MAX(A,B) ((A) >= (B) ? (A) : (B))

/* my constants for GLUT window defaults */
#define MY_WIND_X_SIZE     600
#define MY_WIND_Y_SIZE     600
#define MY_WIND_X_POS      100
#define MY_WIND_Y_POS      100

/* my menu IDs for GLUT menu-handling functions */
#define MY_MENU_EXIT     -9999
#define MY_MENU_RESET     -200
#define MY_MENU_CAMERA      20
#define MY_MENU_LIGHT       30
#define MY_MENU_OBJECT      40
#define MY_ROT_LIGHT        50
#define MY_MENU_PROJECT    100

#define MY_CHECKERS_WIDTH   64
#define MY_CHECKERS_HEIGHT  64


#define   DEGTORAD  (3.14/180.0)

/***************************************************************/
/******************** MY GLOBAL VARIABLES **********************/
/***************************************************************/

/* globals for generated texture - modified from Red Book's original checker.c - */
static GLubyte CheckersImg[MY_CHECKERS_HEIGHT][MY_CHECKERS_WIDTH][3];
static GLuint  TextureName;
static GLboolean TextureOn = GL_TRUE;

static GLboolean key_f1 = GL_TRUE;

static GLboolean key_f2 = GL_TRUE;

static TexMap  TMap;   /* global for texture from readppm */

static GLfloat LightPos0[4];

static GLfloat LightPos1[4];


static GLfloat rot_angle = 0.0;

static GLfloat sunMatrix[16] = {1,0,0,0,  0,1,0,0, 0,0,1,0, 0,0,0,1};

static GLfloat planeMatrix[16] = {1,0,0,0,  0,1,0,0, 0,0,1,0, 0,0,0,1};

static GLfloat azumMatrix[16] = {1,0,0,0,  0,1,0,0, 0,0,1,0, 0,0,0,1};

GLfloat gRotationMatrix[16] = {1, 0, 0, 0,
			       0, 1, 0, 0,
			       0, 0, 1, 0,
			       0, 0, 0, 1};

static GLboolean help_display = GL_FALSE;

/* user interaction */
static GLint InteractionMode;
static GLint XmouseDown, YmouseDown;
static GLint XmouseCurrent, YmouseCurrent;
static GLint XmousePrev, YmousePrev, DX, DY;
static GLint WindHeight, WindWidth;
static GLint KeybModifiers;
static GLint mouseButtons = 0;

/* transformations */
static GLdouble  TX=0, TY=0, TZ=0;

/* 3D Projection */
static GLdouble XCamera, YCamera, ZCamera;
static GLdouble FOVangle, FOVratio, ClipNear, ClipFar;

#define DUALSTEREO    0
#define ACTIVESTEREO  1

static int stereo = DUALSTEREO ;

/****************************camera struct*******************************/

typedef struct{

  float pos[3];
  float look_at[3];
  float up[3];
}Camera;

Camera nose_camera ;
Camera global_camera ;

static float plane_pos[3] = { 0.0, 0.0, 0.0 };

static GLint pov = 1;

/**************************** MAIN *****************************/


int main(int argc, char **argv) {

    glutInit(&argc,argv);

    if( stereo == DUALSTEREO )
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    else
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STEREO);

    glutInitWindowPosition(MY_WIND_X_POS, MY_WIND_Y_POS);
    glutInitWindowSize(MY_WIND_X_SIZE, MY_WIND_Y_SIZE);

    glutCreateWindow("B481 template code with 3D lighting");

    glutDisplayFunc       (myDisplay);
    glutReshapeFunc       (myReshape);
    glutMouseFunc         (myButton);
    glutMotionFunc        (myMotion);
    glutPassiveMotionFunc (NULL);
    glutIdleFunc          (NULL);
    glutKeyboardFunc      (myKeyboard);
    glutVisibilityFunc    (myVisibility);

    fprintf(stderr,"Press ESC or 'q' to Exit.\n");

    myInitGL();
    myInitData(&argc,argv);  /* pass command line arguments for texture */

    /* initialize GLUT menu handler as last one, to take over access
     to GLUT_RIGHT_BUTTON, regardles to what other handlers did to it: */
    myInitMenus();

    glutMainLoop();
    return(EXIT_SUCCESS);
} /* main() */


/* --------------------------------------- */
/*           initialize my data            */
/* --------------------------------------- */

void myInitData(int *argc, char **argv) {

    InteractionMode = MY_MENU_CAMERA;

    TX = 1.1; TY = 0.8; TZ = -2.0;

    XmouseDown = -1;
    YmouseDown = -1;
    XmouseCurrent = XmousePrev = -1;
    YmouseCurrent = YmousePrev = -1;
    DX = DY = 0;
    KeybModifiers = 0;

} /* myInitData() */




/***************************************************************/
/*********************** MOUSE CALLBACKS ***********************/
/***************************************************************/

/* ------------------------------------------------------------------------ */
/* Mouse Button  UP/DOWN  or DOWN/UP transition only */
/* ------------------------------------------------------------------------ */
void myButton(int button, int state, int x, int y) {

    XmouseDown = x;
    YmouseDown = WindHeight - y - 1; /* Invert to right-handed system */

    /* This permits detection of left, middle, left+middle */
    if (state == GLUT_DOWN)
        mouseButtons |= button;
    else
        mouseButtons &= (~(button));

    /* bitmask with GLUT_ACTIVE_SHIFT, GLUT_ACTIVE_CTRL, GLUT_ACTIVE_ALT */
    KeybModifiers = glutGetModifiers();

    if (button == GLUT_LEFT_BUTTON || button == GLUT_MIDDLE_BUTTON) {
        if (state == GLUT_DOWN) {
            XmouseCurrent = XmousePrev = XmouseDown;
            YmouseCurrent = YmousePrev = YmouseDown;
        } else {
            /* at button up, invalidate: */
            XmouseCurrent = XmousePrev = XmouseDown = -1;
            YmouseCurrent = YmousePrev = YmouseDown = -1;
            KeybModifiers = 0;
        }
        DX = DY = 0;
    }
    glutPostRedisplay();
} /* myButton() */


/* ------------------------------------------------------------------------ */
/* Mouse motion WHILE BUTTON DOWN  */
/*  COMPLAINS  if you call glutGetModifiers()  here!! */
/* ------------------------------------------------------------------------ */
void myMotion(int x, int y) {

  float rMDeltaX = 0.0;
  float rMDeltaY = 0.0;


    XmouseCurrent = x;
    YmouseCurrent = WindHeight - y - 1; /* Invert to right-handed system */

    DX = XmouseCurrent - XmousePrev;
    DY = YmouseCurrent - YmousePrev;
    // printf("%d %d \n", DX, DY);

    XmousePrev = XmouseCurrent;
    YmousePrev = YmouseCurrent;

    if(InteractionMode == MY_ROT_LIGHT){
      rMDeltaX = DX * 0.05;
      rMDeltaY = DY * 0.05;
      myRollBall(rMDeltaX, rMDeltaY, gRotationMatrix);
    }

    glutPostRedisplay();
} /* myMotion(int x, int y) */


/* ------------------------------------------------------------------------ */
/* Handle Keyboard */
/* ------------------------------------------------------------------------ */
void myKeyboard(unsigned char key, int x, int y) {
    /*  Option: To force all lower case */
    if (key >= 'A' && key <= 'Z') key = (key-'A') + 'a';

    switch (key) {
    case 'h':
      if(  help_display == GL_TRUE ){
	help_display = GL_FALSE ;
      }
      else {
	help_display = GL_TRUE ;
      }
      fprintf(stderr,"Help: \n"
	      "ESC or q: quit \n"
	      "t: toggle texture\n");
      break;
    case 'q' :
    case  27 :  /* 27 -> ESC */
      fprintf(stderr,"Normal Exit.\n");
      exit(EXIT_SUCCESS);
      break;
    case 'O':
    case 'o':
      if(key_f1){
	key_f1 =GL_FALSE;
	glDisable(GL_LIGHT0);
      } else {
	key_f1 = GL_TRUE;
	glEnable(GL_LIGHT0);
            }
      break;
    case 'P':
    case 'p':
      if(key_f2){
	key_f2 =GL_FALSE;
	glDisable(GL_LIGHT1);
      } else {
	key_f2 = GL_TRUE;
	glEnable(GL_LIGHT1);
      }
      break;

    case 'W':
    case 'w':
      TY += 0.05;

      break;
    case 'S':
    case 's':
      if(TY <= 0.5){
	TY = 0.5;
      }else{
      TY -= 0.05;
      }
      break;
    case 'A':
    case 'a':
      if( rot_angle >= 360.0 ){
	rot_angle = 0.0 ;
      }
      rot_angle  += 5.0;
      break;
    case 'D':
    case 'd':
      if(rot_angle <= 0.0){
	rot_angle = 360.0 ;
      }
	rot_angle -= 5.0;
      break;

    case 'G':
    case 'g':
      pov = 1;
     break;
    case 'V':
    case 'v':
      pov = 2;
      break;
    case 'B':
    case 'b':
      pov = 3;
      break;
        default: fprintf(stderr,"Unhandled key: %c [%d] \n",key,key);
    }

    glutPostRedisplay();

} /* myKeyboard() */



/* -------------------------------------------------------------- */
/* myMenus() - callback for GLUT menus */
/* -------------------------------------------------------------- */
void myMenus (int value) {
  switch(value) {
  case MY_MENU_RESET:
    /* reset things here */
    break;
  case MY_MENU_CAMERA:
    InteractionMode = MY_MENU_CAMERA;
    break;
  case MY_MENU_LIGHT:
    InteractionMode = MY_MENU_LIGHT;
    break;
  case MY_MENU_OBJECT:
    InteractionMode = MY_MENU_OBJECT;
    break;
  case MY_ROT_LIGHT:
    InteractionMode = MY_ROT_LIGHT;
    break;
  case MY_MENU_PROJECT:
    InteractionMode = MY_MENU_PROJECT;
    break;
  case MY_MENU_EXIT:
    exit(0);
    break;
  default:
    //            fprintf(stderr,"unknown menu entry");
    ;
  } /* switch () */
  glutPostRedisplay(); /* make sure we Display according to changes */
} /* myMenus() */


/* -------------------------------------------------------------- */
/* initialize GLUT menus */
/* -------------------------------------------------------------- */
void myInitMenus (void) {
  glutCreateMenu(myMenus);

  glutAddMenuEntry("Move Camera",        MY_MENU_CAMERA);
  glutAddMenuEntry("Move Light",         MY_MENU_LIGHT);
  glutAddMenuEntry("Rotate Light",       MY_ROT_LIGHT);
  glutAddMenuEntry("Move Object",        MY_MENU_OBJECT);
  glutAddMenuEntry(" ", -3);
  glutAddMenuEntry("Exit",               MY_MENU_EXIT);

  glutAttachMenu(GLUT_RIGHT_BUTTON);

} /* myInitMenus() */



/***************************************************************/
/************************** INIT GL ****************************/
/***************************************************************/

void myInitGL (void) {
  int i;
  GLfloat light_ambient0[] = {0.1,0.1,0.1, 1.0};
  GLfloat light_diffuse0[] = {1.0,1.0,1.0, 1.0};
  GLfloat light_specular0[] = {1.0,1.0,1.0, 1.0};


  GLfloat light_ambient1[] = {0.1,0.1,0.0, 1.0};
  GLfloat light_diffuse1[] = {.5,.5,0.0, 1.0};
  GLfloat light_specular1[] = {0.5,0.5,0.0, 1.0};

  /* projection pipeline initialization */
  FOVangle = 90.0;
  FOVratio = 1.0;
  ClipNear = 0.1;
  ClipFar = 1000.0;
  XCamera = YCamera = 0.7;
  ZCamera = 5.0;

  WindWidth = glutGet(GLUT_WINDOW_WIDTH);
  WindHeight  = glutGet(GLUT_WINDOW_HEIGHT);
  glViewport(0, 0, WindWidth, WindHeight);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  FOVratio  = (float)WindWidth / (float)WindHeight; /* keep viewed x/y ratio == 1 */
  /* instead of glFrustum(-1.0*ratio, 1.0*ratio, -1.0,1.0, FLength, 25.0);  */
  gluPerspective(FOVangle, FOVratio, ClipNear, ClipFar);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  /* Colors, Lighting, Material  initialization */
  glClearColor( 0.4f, 0.6f, 0.5f, 1.0f);
  glPointSize(1);     glEnable(GL_POINT_SMOOTH);
  glLineWidth(1);     glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  /* we do **not** want model materials to follow the current GL color: */
  glDisable(GL_COLOR_MATERIAL);

  glShadeModel(GL_SMOOTH);        /* or GL_FLAT */
  glEnable(GL_NORMALIZE);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);

  /* light0 is the spot light */

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient0);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular0);
  LightPos0[0] = -1.0;    LightPos0[1] = 2.0;
  LightPos0[2] =  0.5;    LightPos0[3] = 1.0;


  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient1);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular1);
  LightPos1[0] = 0.0;    LightPos1[1] = 20.0;
  LightPos1[2] =  0.0;    LightPos1[3] = 1.0;

  glLightfv(GL_LIGHT0, GL_POSITION, LightPos0);
  glLightfv(GL_LIGHT1, GL_POSITION, LightPos1);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);

  glutTimerFunc(25, timerCallback, 0);


  i = ReadPPM("data/temp.ppm", &TMap);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TMap.texc, TMap.texr,
	       0, GL_RGB, GL_UNSIGNED_BYTE, TMap.texture);


} /* myInitGL() */



/***************************************************************/
/********************** DRAW FUNCTIONS *************************/
/***************************************************************/


void myDrawTeapot(GLdouble size) {
    GLfloat materialDiffuse[] = {0.7, 0.4, 0.2, 1.0};

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialDiffuse);

    if (TextureOn==GL_TRUE) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBindTexture(GL_TEXTURE_2D, TextureName);
        glFrontFace(GL_CW);
        glutSolidTeapot(size);
        glFrontFace(GL_CCW);
        glDisable(GL_TEXTURE_2D);
    } else {
        glFrontFace(GL_CW);
        glutSolidTeapot(size);
        glFrontFace(GL_CCW);
    }
} /* myDrawTeapot() */



void drawmodel(void){
  if(!pmodel){
    pmodel = glmReadOBJ("data/f-16.obj");
    if(!pmodel) exit(0);
    glmUnitize(pmodel);
    glmFacetNormals(pmodel);
    glmVertexNormals(pmodel, 90.0);
  }
  glmDraw(pmodel, GLM_SMOOTH | GLM_MATERIAL);
}

void drawscene(void){
  if(!smodel){
    smodel = glmReadOBJ("data/scene.obj");
    if(!smodel) exit(0);
    glmUnitize(smodel);
    glmFacetNormals(smodel);
    glmVertexNormals(smodel, 90.0);
  }
  glmDraw(smodel,GLM_SMOOTH);
}


void drawcircle(float radius){
  int i;
  float vert[4];
  float texC[2];
  float angle = 2.0* 3.14/100;

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,1);
  glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
  glBegin(GL_TRIANGLE_FAN);

  texC[0] = 0.5;
  texC[1] = 0.5;
  glTexCoord2fv(texC);

  vert[0] = 0.0;
  vert[1] = 0.0;
  vert[2] = 0.0;
  vert[3] = 1.0;
  glVertex4fv(vert);

  for(i = 0; i < 100; i++){
    texC[0] = (cos(angle*i) + 1.0)/2;
    texC[1] = (sin(angle*i) + 1.0)/2;
    glTexCoord2fv(texC);

    vert[0] = sin(angle*i) * radius;
    vert[1] = cos(angle*i) * radius;
    vert[2] = 0.0;
    vert[3] = 1.0;
    glVertex4fv(vert);
  }

  texC[0] = 1.0;
  texC[1] = 0.5;
  glTexCoord2fv(texC);

  vert[0] = 0.0;
  vert[1] = radius;
  vert[2] = 0.0;
  vert[3] = 1.0;
  glVertex4fv(vert);
  glEnd();

  glDisable(GL_TEXTURE_2D);

}

void vec_mat_mult( GLfloat M[], GLfloat vec[], GLfloat out_vec[]){

  out_vec[0] = M[0]*vec[0] + M[4]*vec[1] + M[8]*vec[2] + M[12]*vec[3] ;
  out_vec[1] = M[1]*vec[0] + M[5]*vec[1] + M[9]*vec[2] + M[13]*vec[3] ;
  out_vec[2] = M[2]*vec[0] + M[6]*vec[1] + M[10]*vec[2] + M[14]*vec[3] ;
  out_vec[3] = M[3]*vec[0] + M[7]*vec[1] + M[11]*vec[2] + M[15]*vec[3] ;

  return;

}


void mat_cross_prod( GLfloat M[], GLfloat N[], GLfloat R[]){

  int i, j, indx;
  float out_vec[4] ;


  for(j=0; j<4; j++){
    vec_mat_mult(M, &N[j*4], out_vec);
    for(i=0; i<4; i++){
      indx = (i * 4)+j;
      R[indx] = out_vec[i] ;
    }
  }

  return ;
}

void vec_cross_prod( float vec1[], float vec2[], float out_vec[] ){

  out_vec[0] = vec1[1]*vec2[2] - vec1[2]*vec2[1];
  out_vec[1] = vec1[2]*vec2[0] - vec1[0]*vec2[2] ;
  out_vec[2] = vec1[0]*vec2[1] - vec1[1]*vec2[0];

  return ;
}



void normalize( float vec[] ){
  float length ;

  length = sqrt( vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2] ) ;

  vec[0] = vec[0]/length ;
  vec[1] = vec[1]/length ;
  vec[2] = vec[2]/length ;

  return ;
}

void pos_nose_camera(void){

  GLfloat M[16] ;
  GLfloat start_pos[] = {0, 0, 0, 1 } ;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glRotatef(rot_angle, 0, 1, 0);
  glTranslatef( TX, TY, TZ) ;
  glGetFloatv(GL_MODELVIEW_MATRIX, M );
  vec_mat_mult( M, start_pos, nose_camera.pos );
  glPopMatrix();
  glPushMatrix();
  glRotatef(rot_angle, 0, 1, 0);
  glGetFloatv(GL_MODELVIEW_MATRIX, M );
  start_pos[0] = nose_camera.look_at[0];
  start_pos[1] = nose_camera.look_at[1];
  start_pos[2] = nose_camera.look_at[2];
  start_pos[3] = nose_camera.look_at[3];
  vec_mat_mult( M, start_pos, nose_camera.look_at );
  glPopMatrix();


  return ;
}


void output(int x, int y, char *string)
{
  int len, i;
  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++)
  {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
  }
}


void draw_scene(){

    /* coordinate system origin - unit vectors
    glDisable(GL_LIGHTING);
    glLineWidth(2.0);
    glBegin(GL_LINES); glColor3f(1.0,0.0,0.0);
    glVertex3f(0.0,0.0,0.0); glVertex3f(1.0,0.0,0.0); glEnd();
    glBegin(GL_LINES); glColor3f(0.0,1.0,0.0);
    glVertex3f(0.0,0.0,0.0); glVertex3f(0.0,1.0,0.0); glEnd();
    glBegin(GL_LINES); glColor3f(0.0,0.0,1.0);
    glVertex3f(0.0,0.0,0.0); glVertex3f(0.0,0.0,1.0); glEnd();
    glEnable(GL_LIGHTING);
    */


    /* lights positioning */
    if (InteractionMode == MY_MENU_LIGHT) {
        /* move light */
        if ((KeybModifiers & GLUT_ACTIVE_SHIFT) == 0) {
            LightPos0[0] = LightPos0[0] + 0.1*(float)DX;
            LightPos0[1] = LightPos0[1] + 0.1*(float)DY;
        } else {
            LightPos0[0] = LightPos0[0] + 0.1*(float)DX;
            LightPos0[2] = LightPos0[2] + 0.1*(float)DY;
        }
    }

    glDisable(GL_LIGHTING);
    glPushMatrix();
    if (InteractionMode == MY_ROT_LIGHT)
    {
	glMultMatrixf(gRotationMatrix);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPos0);
    }
    /* draw a little cube to mark the light's position: */
    glTranslatef(LightPos0[0],LightPos0[1],LightPos0[2]);
    glLightfv(GL_LIGHT0, GL_POSITION, LightPos0);
    glColor3f(1.0,1.0,1.0);
    glutSolidSphere(0.1, 50, 50);
    glPopMatrix();
    glEnable(GL_LIGHTING);


    /* now draw the sun */
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glMultMatrixf(sunMatrix);
    glTranslatef(LightPos1[0],LightPos1[1],LightPos1[2]);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPos1);
    glColor3f(1.0,1.0,0.0);
    glutSolidSphere(0.75, 50, 50);
    glPopMatrix();
    glEnable(GL_LIGHTING);

    /* draw the aircraft */
    glPushMatrix();
    glMultMatrixf(planeMatrix);
    glTranslatef(plane_pos[0],plane_pos[1],plane_pos[2]);
    glRotatef(rot_angle, 0, 1, 0);
    glScalef(0.35, 0.35, 0.35);
    drawmodel();
    glPopMatrix();

    /* draw the scene */
    glPushMatrix();
    glScalef(15.0, 8.5, 15.5);
    drawscene();
    glPopMatrix();


    /* draw textured circle */
    glPushMatrix();
    glTranslatef(-8, -1.35, -0.75);
    glRotatef(90, -1, 0, 0);
    drawcircle(5.5);
    glPopMatrix();



    return ;
}

static float eyesep = 0.05 ;
static float near = 10 ;
static float far = 2000 ;
static float focallength = 100 ;
static float aperture = 60.0 ;

/* ------------------------------------------------------------------------ */
void myDisplay(void) {
  double ratio,radians,wd2,ndfl;
  double left,right,top,bottom;
  int wind_hi, wind_wid ;
  float r[4], vec1[4], vec2[4] ;

  wind_hi =  glutGet(GLUT_WINDOW_HEIGHT) ;
  wind_wid =  glutGet(GLUT_WINDOW_WIDTH) ;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   /* Misc stuff needed for the frustum */
   ratio   = (double)wind_wid / (double)wind_hi;
   radians = DEGTORAD * aperture / 2;
   wd2     = near * tan(radians);
   ndfl    = near / focallength;
   top     =   wd2;
   bottom  = - wd2;

    if(pov == 1 )
      {
        vec1[0] = 0 ;
        vec1[1] = 0 ;
        vec1[2] = -1;
        vec2[0] = 0 ;
        vec2[1] = 1 ;
        vec2[2] = 0;
        vec_cross_prod(vec1,vec2,r);
      }
    else if(pov == 3)
      {
        vec1[0] = nose_camera.pos[0] ;
        vec1[1] = 0 ;
        vec1[2] = nose_camera.pos[2];
        vec2[0] = 0 ;
        vec2[1] = 0 ;
        vec2[2] = -1;
        vec_cross_prod(vec1,vec2,r);
      }
    else
      {
        vec1[0] = nose_camera.look_at[0] ;
        vec1[1] = nose_camera.look_at[1] ;
        vec1[2] = nose_camera.look_at[2];
        vec2[0] = 0 ;
        vec2[1] = 1 ;
        vec2[2] = 0;
        vec_cross_prod(vec1,vec2,r);
      }
   normalize(r);
   r[0] *= eyesep / 2.0;
   r[1] *= eyesep / 2.0;
   r[2] *= eyesep / 2.0;



      if (stereo == DUALSTEREO)
         glViewport(wind_wid/2,0,wind_wid/2,wind_hi);
      else
         glViewport(0,0,wind_wid,wind_hi);
      glMatrixMode(GL_MODELVIEW);
      if (stereo == DUALSTEREO) {
         glDrawBuffer(GL_BACK);
      }else{
         glDrawBuffer(GL_BACK_RIGHT);
      }
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glLoadIdentity();

    if(pov == 1 )
      {
	    gluLookAt(0+r[0], 8+r[1], 0+r[2], 0+r[0], 0+r[1], -1+r[2], 0, 1, 0);
      }
    else if(pov == 3)
      {
		gluLookAt(nose_camera.pos[0]+r[0], 8+r[1], nose_camera.pos[2]+r[2],
                            nose_camera.pos[0]+r[0], 0+r[1], nose_camera.pos[2]+r[2],
                            0, 0, -1);
      }
    else
      {
	gluLookAt( nose_camera.pos[0]+r[0], nose_camera.pos[1]+r[1], nose_camera.pos[2]+r[2],
	            nose_camera.look_at[0]+r[0], nose_camera.look_at[1]+r[1], nose_camera.look_at[2]+r[2],
                    0,1,0 );
      }

      draw_scene() ;
      //glMatrixMode(GL_PROJECTION);
      //glPopMatrix();


      //glMatrixMode(GL_PROJECTION);
      //glPushMatrix();
      //glLoadIdentity();
      //left  = - ratio * wd2;
      //right =   ratio * wd2;

      //left  = - ratio * wd2 + 0.5 * eyesep * ndfl;
      //right =   ratio * wd2 + 0.5 * eyesep * ndfl;
      //glFrustum(left,right,bottom,top,near,far);


      if (stereo == DUALSTEREO)
         glViewport(0,0,wind_wid/2,wind_hi);
      else
         glViewport(0,0,wind_wid,wind_hi);
      glMatrixMode(GL_MODELVIEW);
      if (stereo == DUALSTEREO) {
         glDrawBuffer(GL_BACK);
      }else{
         glDrawBuffer(GL_BACK_LEFT);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      }
      glLoadIdentity();

    if(pov == 1 )
      {
	     gluLookAt(0-r[0], 8-r[1], 0-r[2], 0-r[0], 0-r[1], -1-r[2], 0, 1, 0);
      }
    else if(pov == 3)
      {
		gluLookAt(nose_camera.pos[0]-r[0], 8-r[1], nose_camera.pos[2]-r[2],
                            nose_camera.pos[0]-r[0], 0-r[1], nose_camera.pos[2]-r[2],
                            0, 0, -1);
      }
    else
      {
	gluLookAt( nose_camera.pos[0]-r[0], nose_camera.pos[1]-r[1], nose_camera.pos[2]-r[2],
	            nose_camera.look_at[0]-r[0], nose_camera.look_at[1]-r[1], nose_camera.look_at[2]-r[2],
                    0,1,0 );
      }


      draw_scene() ;
      //glMatrixMode(GL_PROJECTION);
      //glPopMatrix();

    glutSwapBuffers();
} /* myDisplay() */


/***************************************************************/
/*********************** GLUT FUNCTIONS ************************/
/***************************************************************/


/* ------------------------------------------------------------------------ */
void myReshape(int w, int h) {
    w = MY_MAX(w,128);
    h = MY_MAX(h,128);
    WindHeight = h;
    WindWidth = w;
    glutReshapeWindow(w,h);
    glViewport(0,0,w,h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    FOVratio  = (float)w / (float)h; /* keep world coord x/y ratio == 1 */
    gluPerspective(FOVangle, FOVratio, ClipNear, ClipFar);
    glMatrixMode(GL_MODELVIEW);
} /* myReshape() */

/* ------------------------------------------------------------------------ */
void myVisibility(int state) {
    /* ------------------------------------------------------------------------ */
    if (state == GLUT_VISIBLE){
        glutPostRedisplay();
    }
} /* myVisibility() */




void myRollBall (GLfloat pDeltaX, GLfloat pDeltaY, GLfloat *pModelMatrix ) {

    GLfloat rotateR = sqrt(pDeltaX*pDeltaX + pDeltaY*pDeltaY);

    /* only rotate when there is a non-zero angle! */
    if (rotateR > 0.01) {
      normal[0] = -pDeltaY/rotateR;
      normal[1] = pDeltaX/rotateR;
      float theta = atan2(pDeltaY, pDeltaX);

      theta = rotateR ;//theta*180.0/M_PI;
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glRotatef(theta, normal[0], normal[1], normal[2]);
      glMultMatrixf(pModelMatrix);
      glGetFloatv(GL_MODELVIEW_MATRIX, pModelMatrix);
    }
     glutPostRedisplay();

} /* myRollBall() */

#define   SPEED     0.05
static float  sun_angle = 0.0 ;

static void timerCallback (int value) {

  TX = SPEED * sin( rot_angle * DEGTORAD ) ;
  TZ = SPEED * cos( rot_angle * DEGTORAD ) ;



  plane_pos[0] += TX ;
  plane_pos[1] = TY ;
  plane_pos[2] += TZ ;

  nose_camera.pos[0] += TX ;
  nose_camera.pos[1] = TY ;
  nose_camera.pos[2] += TZ ;
  nose_camera.look_at[0] =  plane_pos[0] + TX ;
  nose_camera.look_at[1] = TY - .05;
  nose_camera.look_at[2] =  plane_pos[2] + TZ ;


  glMatrixMode(GL_MODELVIEW);
  glPushMatrix() ;
  glLoadIdentity() ;
  glTranslatef(TX, TY, TZ);
  glGetFloatv(GL_MODELVIEW_MATRIX, planeMatrix);
  glPopMatrix();

  if(sun_angle >= 360){
    sun_angle = 0.0;
  } else {
    sun_angle += 1.0;
  }

  glPushMatrix();
  glLoadIdentity();
  glRotatef(sun_angle, 0.0, 0.0, 1.0);
  glGetFloatv(GL_MODELVIEW_MATRIX, sunMatrix);
  glPopMatrix();

  glutPostRedisplay();

  glutTimerFunc(25, timerCallback, 0);

}


/* ------------------------------------------------------------------------ */
