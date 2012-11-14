/* Include file readoff.h for use
   with readoff.c and other geometry utilities
 */

/* Geometric objects */

typedef struct {
    double x,y;
    } Point2;

typedef struct {
  double x, y, z;
} Point3;

typedef struct {
  double x, y, z, w;
} HPoint3;

typedef struct {
  int Nvertices;                /* number of total vertices */
  int Nfaces;                   /* number of total faces */
  int Nedges;                   /* (ignored) number of total edges */
  HPoint3 * vertices;           /* coordinates of each vertex */
  int *nv_face;                 /* number of vertices for each face */
  int **faces;                  /* faces[i][j] is the index of face[i]'s
                                   j-th vertex */
} Object3D;

/* 3-by-3 matrix */
typedef struct  { 
        double elem[3][3];  } Matrix3;

/* 4-by-4 matrix */
typedef struct  { 
        double elem[4][4];  } Matrix4;


typedef Point2 Vector2;
typedef Point3 Vector3;
typedef HPoint3 HVector3;


/*********************************************************************/
/* function prototypes */
/*********************************************************************/
double V3squaredLength(Vector3 a);
double V3Length(Vector3 a);
Vector3 V3Normalize(Vector3 a);
Vector3 V3add(Vector3 a, Vector3 b);
Vector3 V3diff(Vector3 a, Vector3 b);
double V3dot(Vector3 a, Vector3 b);
Vector3 V3cross(Vector3 a, Vector3 b);
Matrix4 CopyMatrix3D(Matrix4 InMat);
Matrix4 Mult4(Matrix4 a, Matrix4 b);
HPoint3 TransHPoint3(Matrix4 m, HPoint3 p);
HPoint3 Homogenize(HPoint3 a);
void PrintMat(Matrix4 m);
void PrintHPoint(HPoint3 p);
void PrintPoint(Point3 p);
Matrix4 IdentMatrix();
Matrix4 SetScaleMatrix(double sx,double sy,double sz);
Matrix4 SetTransMatrix(double tx,double ty,double tz);
Matrix4 SetRotMatrix(Vector3 n,double angle);
Point3 HPt3toPt3(HPoint3 pt);
HPoint3 Pt3toHPt3(Point3 pt);
HPoint3 ScaleHPoint3(double s, HPoint3 a);
