
#include "nfd.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <iostream>
#include <sstream>
#include <string.h>
#include <GLUI/glui.h>
#include <GLUT/GLUT.h>
#include "glm/glm.hpp"
#include "glm/gtx/component_wise.hpp"

using namespace std;

//the follow is used to the glui controls and transformation controls
#define IMPORT_ID            100
#define DISPLAY_ID           103
#define PROJECTION_ID        104
#define CAMERA_ID            105
#define LIGHT_ID             106

#define DISPLAY_POINTS       300
#define DISPLAY_WIRE         301
#define DISPLAY_FLAT         302
#define DISPLAY_SMOOTH       303

#define PROJECTION_ORO       0
#define PROJECTION_PER       1
#define TRANSFORM_ROTATE     10
#define TRANSFORM_TRANSLATE  11
#define TRANSFORM_SCALE      12
#define TRANSFORM_NONE       13

#define THREE_D              20
#define YZ                   21
#define XZ                   22
#define XY                   23

#define LIGHT0_ENABLED_ID    200
#define LIGHT1_ENABLED_ID    201
#define LIGHT0_INTENSITY_ID  250
#define LIGHT1_INTENSITY_ID  260

#define LIGHT0_DIFFUSE       400
#define LIGHT0_AMBIENT       401
#define LIGHT0_SPECULAR      402





//the following variables are used to display the model
GLfloat xy_aspect,axe_x=0.0f,axe_y=0.0f,axe_z=0.0f;

int load=0,reload=0;
int v_num=0,f_num=0;
int display_mode = DISPLAY_SMOOTH;
int projection_mode = PROJECTION_PER;
int camera_mode = THREE_D;
int light1_mode = LIGHT0_DIFFUSE;
float center[3];

GLfloat B_r=0.9f,B_g=0.9f,B_b=0.9f;
GLfloat r = 1.0f, g = 0.0f, b = 1.0f;
GLUquadricObj *quadratic;
int index_vertex,index_face;
float Vnormal =1;


int   light0_enabled = 1;
int   light1_enabled = 1;
float light0_intensity = 1.0;
float light1_intensity = .4;


const char *path1 = "" ;




//Meterial
GLfloat m_ambient[] =  {0.4f, 0.4f, 0.3f, 1.0f};
GLfloat m_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
GLfloat m_specular[] =  {0.9f, .6f, 0.9f, 1.0f};
GLfloat m_shininess[] = {40};
//LIGHT
GLfloat light0_ambient[] =  {0.4f, 0.4f, 0.3f, 1.0f};
GLfloat light0_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
GLfloat light0_specular[] =  {1.0f, .6f, 1.0f, 1.0f};
GLfloat light0_position[] = {.5f, .5f, 1.0f, 0.0f};
GLfloat light0_shininess[] = {40};
//GLfloat light0_emission[] = {0.3, 0.2, 0.2, 0.0};

GLfloat light1_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
GLfloat light1_diffuse[] =  {.9f, .6f, 0.0f, 1.0f};
GLfloat light1_position[] = {-1.0f, -1.0f, 1.0f, 0.0f};

GLfloat lights_rotation[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
//GLfloat lights_rotation2[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

GLfloat temp[] =  {.6f, .6f, 1.0f, 1.0f};;








// directional light (w=0)
GLfloat light0_position1[] = {1., 1., 5., 0.};

GLUI_Spinner    *light0_spinner, *light1_spinner;

GLUI_Panel      *obj_panel;

//this struct defines a vertex
typedef struct
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
    //normal vector
    // GLfloat n[3];
} vertex;

//this struct defines a face
typedef struct
{
    int v1;
    int v2;
    int v3;
} face;

struct HE_edge;
struct HE_vert;
struct HE_face;


struct HE_edge {
    HE_vert* vert;
    HE_edge* pair;
    HE_face* face;
    HE_edge* prev;
    HE_edge* next;
};
struct HE_vert {
    float x, y, z;
    HE_edge* edge;
    vector<HE_edge*> temp;
    vector< glm::vec3> temp2;
    GLfloat n[3];
};
struct HE_face {
    int v1,v2,v3;
    HE_edge* edge; // one of the half-edges bordering the face };
};



HE_vert vert[150000];//used to store vertex information
HE_face face1[300000];  //used to store face information
HE_edge edge[300000];

vertex v_up[4]; //used to store the up 4 points of bounding box
vertex v_down[4];//used to store the down 4 points of bounding box



//the following live values are passed into GLUI window
int main_window;
int draw_axe=1;
int draw_grid=1;
int draw_box=0;
int object_only = 0;
int show_text = 0;


// Pointers to the windows and some of the controls we'll create **/
GLUI *sub_window,*sub_window1,*sub_window2;
GLUI_Panel *display_panel,*projection_panel,*import_panel,*cor_system,*camera ,*light;
GLUI_RadioGroup *radio_group,*radio_group1,*camera_group, *light_group;
GLUI_Scrollbar *sb, *sb1;
GLUI_Rotation   *lights_rot;
//The following variables are used to transform the world coordination system
//and the model's local coordination system
GLfloat x_angle=-30,y_angle,x_dis=0,y_dis=-1.0f,scale_size=2,old_size;
GLfloat x_angle_model=0,y_angle_model=0,x_dis_model=0,y_dis_model=0,scale_size_model=1;
int mouse_x,mouse_y;
int transform_mode;


//This function read the vertex and face information from the m file named 'filename'
//and store the vertex and face information in vecter 'vertex' and 'face'.
void import(const char *path, HE_vert vert[],HE_face face1[], int* vertex_num, int* face_num,HE_edge edge[])
{
    memset(vert, 0x0, 50000 * sizeof(HE_vert));
    memset(face1, 0x0, 90000 * sizeof(HE_face));
    memset(edge, 0x0, 90000 * sizeof(HE_edge));
    FILE *file;
    char first;
    int v1,v2,v3;
    float x,y,z;
    //open file
    file=fopen(path,"r");
    int j = 0;
    glm::vec3  Normal;
    while((first=fgetc(file))!= EOF)
    {
        switch (first)
        {
            case 'V': if ( fscanf(file,"ertex %d %f %f %f\n",
                                  &index_vertex,&x,&y,&z) == 4 )
            {
                
                //store vertex information
                vert[index_vertex-1].x=(GLfloat)x;
                vert[index_vertex-1].y=(GLfloat)y;
                vert[index_vertex-1].z=(GLfloat)z;
                
                
            }
                break;
            case 'F': if ( fscanf(file,"ace %d %d %d %d\n",&index_face,&v1,&v2,&v3) == 4)
            {
                //store face information
                face1[index_face-1].v1=v1;
                face1[index_face-1].v2=v2;
                face1[index_face-1].v3=v3;
                
                
            }
                break;
            case '#': do
            { first =fgetc(file);
            }while( first != '\n' && first != EOF);
                break;
        }
    }
    //store the number of vertex and the number of face
    *vertex_num = index_vertex;
    *face_num = index_face;
    f_num =index_face;
    //read test
    
    for(int i=0; i <= index_vertex-1;i++){
        
        //    printf("%d...x:%f,y:%f,z:%f\n",i+1,vert[i].x,vert[i].y,vert[i].z);
        //    printf("face %d : %d,%d,%d",i,face1[i].v1,face1[i].v2,face1[i].v3);
    }
    
    for(int i=0; i <= index_face-1;i++){
        //  HE_face * face1 = new HE_face;
        //    HE_vert * vert = new HE_vert;
        HE_edge * edge1 = new HE_edge;
        HE_edge * edge2 = new HE_edge;
        HE_edge * edge3 = new HE_edge;
        
        
        //   edge1->vert = &vert[face1[index_face].v1];
        edge1->face = &face1[i];
        edge1->prev = edge3;
        edge1->next = edge2;
        
        if (edge1->vert == NULL){
            edge1->vert = &vert[face1[i].v1-1];
        }
        
        edge2->face = &face1[i];
        edge2->prev = edge1;
        edge2->next = edge3;
        
        if (edge2->vert == NULL){
            edge2->vert = &vert[face1[i].v2-1];
        }
        
        
        edge3->face = &face1[i];
        edge3->prev = edge2;
        edge3->next = edge1;
        
        if (edge3->vert == NULL){
            edge3->vert = &vert[face1[i].v3-1];
        }
        
        
        
        edge[i].face =&face1[i];
        face1[i].edge = edge1;
        edge[i].next = edge2;
        edge[i].prev = edge3;
        edge[i].vert = &vert[face1[i].v1-1];
        
        vert[face1[i].v1-1].edge = edge1;
        
        
        
        vert[face1[i].v1-1].temp.push_back(edge1);
        vert[face1[i].v1-1].temp.push_back(edge2);
        
        vert[face1[i].v2-1].temp.push_back(edge2);
        vert[face1[i].v2-1].temp.push_back(edge3);
        
        vert[face1[i].v3-1].temp.push_back(edge3);
        vert[face1[i].v3-1].temp.push_back(edge1);
        
        
        glm::vec3  u(vert[face1[i].v2-1].x - vert[face1[i].v1-1].x,vert[face1[i].v2-1].y - vert[face1[i].v1-1].y,vert[face1[i].v2-1].z - vert[face1[i].v1-1].z);
        glm::vec3  v(vert[face1[i].v3-1].x - vert[face1[i].v1-1].x,vert[face1[i].v3-1].y - vert[face1[i].v1-1].y,vert[face1[i].v3-1].z - vert[face1[i].v1-1].z);
        
        
        
        vert[face1[i].v1-1].temp2.push_back(glm::normalize( glm::cross( u, v ) ));
        
        vert[face1[i].v2-1].temp2.push_back(glm::normalize( glm::cross( u, v ) ));
        
        vert[face1[i].v3-1].temp2.push_back(glm::normalize( glm::cross( u, v ) ));
        
        
        //   printf("vert of half-edge : %f,%f,%f\n", edge[i].vert->x, edge[i].vert->y, edge[i].vert->z);
        
        
        
        
    }
    
    
    for(int i=0; i <= index_face-1;i++){
        
        int t =vert[face1[i].v1-1].temp.size();
        //  printf("vert of piar,no.%dn", t);
        
        for(j=0;j<t;j++){
            if(edge[i].prev->vert == vert[face1[i].v1-1].temp[j]->vert){
                edge[i].pair = vert[face1[i].v1-1].temp[j];
            }
            
            
        }
        
    }
    int t2 =vert[0].temp2.size();

            for(j=0;j<t2+1;j++){
  //  printf("Normal->x:%f,y:%f,z:%f\n",vert[0].temp2[j].x,vert[0].temp2[j].y,vert[0].temp2[j].z);
            }
    for(int i=0; i <= index_vertex-1;i++){
        int t1 =vert[i].temp2.size();
        glm::vec3  V_normal;
        for(j=0;j<t1;j++){
        //      printf("No.%d:normals of vert,no.%d\n", i,t1);
            
            V_normal += vert[i].temp2[j];
        }
  //      printf("Normal->x:%f,y:%f,z:%f\n",V_normal.x,V_normal.y,V_normal.z);

        glm::vec3 temp_c (V_normal.x/t1, V_normal.y/t1,V_normal.z/t1);
        glm::vec3 temp_c2( glm::normalize(temp_c));
        vert[i].n[0] = temp_c2.x;
        vert[i].n[1] = temp_c2.y;
        vert[i].n[2] = temp_c2.z;
        

        
        
    }
//    printf("Normal->x:%f,y:%f,z:%f",vert[1].n[0],vert[1].n[1],vert[1].n[2]);
    
    //  firsr face(32410,10377,10380) : first point : Vertex 32410  0.0854014 0.00632584 0.0580443
    //second point :Vertex 10377  0.08517 0.00555531 0.0571252;
    //third point :Vertex 10380  0.0860727 0.00618433 0.0575728;
    //   for(int i=0; i <= index_face-1;i++){
    
 //   printf("vert of piar,no.%d : %f,%f,%f\n", 0,edge[0].pair->vert->x, edge[0].vert->y, edge[0].vert->z);
  

    
    
}

float Max(float a,float b,float c)
{
    float maxup1 =a>b?a:b;
    float maxup2 =c>maxup1?c:maxup1;
    return maxup2;
}



//This function find the 8 bounding points of a model v
//and store the points into v_up and v_down
void findedge(HE_vert vert[],vertex v_up[],vertex v_down[])
{
    
    int i;
    GLfloat max_x,max_y,max_z;
    GLfloat min_x,min_y,min_z;
    max_x=max_y=max_z = -10.0f;
    min_x=min_y=min_z = 10.0f;
    for(i=0;i<v_num;i++)
    {
        //find the minimum of x and store
        if(vert[i].x<min_x)
        {
            v_up[0].x = vert[i].x;
            v_up[3].x  = vert[i].x;
            v_down[0].x  = vert[i].x;
            v_down[3].x  = vert[i].x;
            min_x = vert[i].x;
        }
        //find the minimum of y and store
        if(vert[i].y<min_y)
        {
            v_down[0].y = vert[i].y;
            v_down[1].y = vert[i].y;
            v_down[2].y = vert[i].y;
            v_down[3].y = vert[i].y;
            min_y = vert[i].y;
        }
        //find the minimum of z and store
        if(vert[i].z<min_z)
        {
            v_up[0].z = vert[i].z;
            v_up[1].z = vert[i].z;
            v_down[0].z = vert[i].z;
            v_down[1].z = vert[i].z;
            min_z = vert[i].z;
        }
        //find the maximum of x and store
        if(vert[i].x>max_x)
        {
            v_up[1].x = vert[i].x;
            v_up[2].x = vert[i].x;
            v_down[1].x = vert[i].x;
            v_down[2].x = vert[i].x;
            max_x = vert[i].x;
        }
        //find the maximum of y and store
        if(vert[i].y>max_y)
        {
            v_up[0].y = vert[i].y;
            v_up[1].y = vert[i].y;
            v_up[2].y = vert[i].y;
            v_up[3].y = vert[i].y;
            max_y = vert[i].y;
        }
        //find the maximum of z and store
        if(vert[i].z>max_z)
        {
            v_up[2].z = vert[i].z;
            v_up[3].z = vert[i].z;
            v_down[2].z = vert[i].z;
            v_down[3].z = vert[i].z;
            max_z = vert[i].z;
        }
    }
    
    
    
    
    for(int i=0;i<10;i++){
        if( Max(max_x,max_y,max_z)/Vnormal>5){
            Vnormal = Vnormal *10;
        }
        if( Max(max_x,max_y,max_z)/Vnormal<1){
            Vnormal = Vnormal *0.1;
        }
        
    }
    // printf("normal--scale:%f",Vnormal);
    
    
    
    
}

//This function set the bounding point whose x,y,z are
//minimum as the original point of the coordinate system
void set( HE_vert vert[], vertex v_down[])
{
    
    int i;
    for(i=0;i<v_num;i++)
    {
        vert[i].x = vert[i].x - v_down[0].x;
        vert[i].y = vert[i].y - v_down[0].y;
        vert[i].z = vert[i].z - v_down[0].z;
    }
}

//This function draw the 3D model
void Draw_model()
{
    
    int i;
    //draw the 3D model as points
    if( display_mode == DISPLAY_POINTS )
    {
        glColor3f(0.0f,0.0f,0.0f);
        for(i=0;i<v_num;i++)
        {
            glBegin(GL_POINTS);
            glVertex3f(vert[i].x/Vnormal-center[0]/Vnormal,vert[i].y/Vnormal-center[1]/Vnormal,vert[i].z/Vnormal-center[2]/Vnormal);
            glEnd();
        }
        glEnable(GL_LIGHTING);
    }
    //draw the 3D model at flat shading model
    else if( display_mode == DISPLAY_FLAT )
    {
        glColor3f(0.0f,0.0f,0.0f);
        for(i=0;i<f_num;i++)
        {
            glBegin(GL_TRIANGLES);
            glNormal3f(vert[face1[i].v1-1].n[0],vert[face1[i].v1-1].n[1],vert[face1[i].v1-1].n[2]);
            
            glVertex3f(vert[face1[i].v1-1].x/Vnormal-center[0]/Vnormal,vert[face1[i].v1-1].y/Vnormal-center[1]/Vnormal,vert[face1[i].v1-1].z/Vnormal-center[2]/Vnormal);
            
            glNormal3f(vert[face1[i].v2-1].n[0],vert[face1[i].v2-1].n[1],vert[face1[i].v2-1].n[2]);
            
            glVertex3f(vert[face1[i].v2-1].x/Vnormal-center[0]/Vnormal,vert[face1[i].v2-1].y/Vnormal-center[1]/Vnormal,vert[face1[i].v2-1].z/Vnormal-center[2]/Vnormal);
            
            glNormal3f(vert[face1[i].v3-1].n[0],vert[face1[i].v3-1].n[1],vert[face1[i].v3-1].n[2]);
            
            glVertex3f(vert[face1[i].v3-1].x/Vnormal-center[0]/Vnormal,vert[face1[i].v3-1].y/Vnormal-center[1]/Vnormal,vert[face1[i].v3-1].z/Vnormal-center[2]/Vnormal);
            glEnd();
        }
        glColor3f(r,g,b);
    }
    //draw the 3D model at smooth shading model
    else if( display_mode == DISPLAY_SMOOTH )
    {
        glColor3f(0.0f,0.0f,0.0f);
        for(i=0;i<f_num;i++)
        {
            glBegin(GL_TRIANGLES);
            glNormal3f(vert[face1[i].v1-1].n[0],vert[face1[i].v1-1].n[1],vert[face1[i].v1-1].n[2]);
            
            glVertex3f(vert[face1[i].v1-1].x/Vnormal-center[0]/Vnormal,vert[face1[i].v1-1].y/Vnormal-center[1]/Vnormal,vert[face1[i].v1-1].z/Vnormal-center[2]/Vnormal);
            
            glNormal3f(vert[face1[i].v2-1].n[0],vert[face1[i].v2-1].n[1],vert[face1[i].v2-1].n[2]);
            
            glVertex3f(vert[face1[i].v2-1].x/Vnormal-center[0]/Vnormal,vert[face1[i].v2-1].y/Vnormal-center[1]/Vnormal,vert[face1[i].v2-1].z/Vnormal-center[2]/Vnormal);
            
            glNormal3f(vert[face1[i].v3-1].n[0],vert[face1[i].v3-1].n[1],vert[face1[i].v3-1].n[2]);
            
            glVertex3f(vert[face1[i].v3-1].x/Vnormal-center[0]/Vnormal,vert[face1[i].v3-1].y/Vnormal-center[1]/Vnormal,vert[face1[i].v3-1].z/Vnormal-center[2]/Vnormal);
            glEnd();
        }
        glColor3f(r,g,b);
    }
    //draw the 3D model as a wireframe
    else if( display_mode == DISPLAY_WIRE )
    {
        glColor3f(0.0f,0.0f,0.0f);
        for(i=0;i<f_num;i++)
        {
            glBegin(GL_LINE_LOOP);
            glNormal3f(vert[face1[i].v1-1].n[0],vert[face1[i].v1-1].n[1],vert[face1[i].v1-1].n[2]);
            
            glVertex3f(vert[face1[i].v1-1].x/Vnormal-center[0]/Vnormal,vert[face1[i].v1-1].y/Vnormal-center[1]/Vnormal,vert[face1[i].v1-1].z/Vnormal-center[2]/Vnormal);
            
            glNormal3f(vert[face1[i].v2-1].n[0],vert[face1[i].v2-1].n[1],vert[face1[i].v2-1].n[2]);
            
            glVertex3f(vert[face1[i].v2-1].x/Vnormal-center[0]/Vnormal,vert[face1[i].v2-1].y/Vnormal-center[1]/Vnormal,vert[face1[i].v2-1].z/Vnormal-center[2]/Vnormal);
            
            glNormal3f(vert[face1[i].v3-1].n[0],vert[face1[i].v3-1].n[1],vert[face1[i].v3-1].n[2]);
            
            glVertex3f(vert[face1[i].v3-1].x/Vnormal-center[0]/Vnormal,vert[face1[i].v3-1].y/Vnormal-center[1]/Vnormal,vert[face1[i].v3-1].z/Vnormal-center[2]/Vnormal);
            glEnd();
        }
        glEnable(GL_LIGHTING);
    }
    else
    {};
}





//This function is a callback of glut used
//to control what to display on the window
void Display( void )
{
    for(int i=0;i<4;i++){
        center[0] += v_up[i].x+v_down[i].x;
        center[1] += v_up[i].y+v_down[i].y;
        center[2] += v_up[i].z+v_down[i].z;
    }
    center[0] =center[0]/8;
    center[1] =center[1]/8;
    center[2] =center[2]/8;
    
    GLfloat j;
    glClearColor( .9f, .9f, .9f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -xy_aspect*.04, xy_aspect*.04, -.04, .04, .1, 15.0 );
    
    glMatrixMode( GL_MODELVIEW );
    
    glLoadIdentity();
    glMultMatrixf( lights_rotation );
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    
    //meterial
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, m_shininess);
    //glLoadIdentity();

    

    
 //   glLoadIdentity();
  //  glMultMatrixf( lights_rotation2 );
 //   glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    
    //enable depth test
    glEnable(GL_DEPTH_TEST);
    //choose shading mode according to the user's choice
    if( display_mode == DISPLAY_FLAT )
        glShadeModel(GL_FLAT);
    else if( display_mode == DISPLAY_SMOOTH )
        glShadeModel(GL_SMOOTH);
    //choose projection mode according to the user's choice
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    if(projection_mode == PROJECTION_ORO)
    {
        glOrtho(-2.5f*xy_aspect,2.5f*xy_aspect,-2.5f,2.5f,0.1f,100.0f);
    }
    if(projection_mode == PROJECTION_PER)
    {
        gluPerspective(60.0f, xy_aspect, 0.1f, 100.0f);
    }
    //set up view mode
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    
    
    if ( camera_mode == THREE_D  ){
        //set the camera
        gluLookAt(center[0]/Vnormal,center[1]/Vnormal,center[2]/Vnormal+5,0,0,0,0,1,0);
        //translate,rotate,scale the model
        glTranslatef(x_dis,y_dis,0.0f);
        glRotatef(x_angle, 0, 1,0);
        // printf("x_angle:%f",x_angle);
        glRotatef(y_angle, 1,0,0);
        //printf("y_angle:%f",y_angle);
        
    }
    if ( camera_mode == YZ ){
        
        //set the camera
        gluLookAt(5.0f,0,0,0,0,0,0,1,0);
        
    }
    if ( camera_mode == XZ){
        //set the camera
        gluLookAt(0,5.0f,0,0,0,0,0,0,1);
    }
    if ( camera_mode == XY ){
        //set the camera
        gluLookAt(0,0,5.0f,0,0,0,0,1,0);
    }
    
    
    
    glScalef(scale_size, scale_size, scale_size);
    
    //set object color;
    glColor3f(r,g,b);
    
    //draw the x,y,z axes
    if( draw_axe == 1)
    {
        glDisable(GL_LIGHTING);
        //draw axe z
        glColor3f( 0.0f,0.0f,1.0f );
        quadratic=gluNewQuadric();
        gluQuadricNormals(quadratic, GLU_SMOOTH);
        gluQuadricTexture(quadratic, GL_FALSE);
        gluCylinder(quadratic,0.008f,0.008f,1.5,32,32);
        glPushMatrix();
        glTranslatef( 0.0f,0.0f,1.5);
        gluCylinder(quadratic,0.04f,0.0f,0.1f,32,32);
        glPopMatrix();
        
        //draw axe x
        glColor3f( 1.0f,0.0f,0.0f);
        glPushMatrix();
        glRotatef(90.0f, 0.0f,1.0f,0.0f);
        quadratic=gluNewQuadric();
        gluQuadricNormals(quadratic, GLU_SMOOTH);
        gluQuadricTexture(quadratic, GL_FALSE);
        gluCylinder(quadratic,0.008f,0.008f,1.5,32,32);
        glPushMatrix();
        glTranslatef( 0.0f,0.0f,1.5);
        gluCylinder(quadratic,0.04f,0.0f,0.1f,32,32);
        glPopMatrix();
        glPopMatrix();
        
        //draw axe y
        glColor3f( 0.0f,1.0f,0.0f);
        glPushMatrix();
        glRotatef(-90.0f, 1.0f,0.0f,0.0f);
        quadratic=gluNewQuadric();
        gluQuadricNormals(quadratic, GLU_SMOOTH);
        gluQuadricTexture(quadratic, GL_FALSE);
        gluCylinder(quadratic,0.008f,0.008f,1.5,32,32);
        glPushMatrix();
        glTranslatef( 0.0f,0.0f,1.5);
        gluCylinder(quadratic,0.04f,0.0f,0.1f,32,32);
        glPopMatrix();
        glPopMatrix();
        glColor3f(r,g,b);
        glEnable(GL_LIGHTING);
        
        /*
         glDisable( GL_LIGHTING );
         glLineWidth( 5.0f );
         glPushMatrix();
         //   glScalef( scale_size_model, scale_size_model, scale_size_model );
         
         glBegin( GL_LINES );
         
         glColor3f( 1.0, 0.0, 0.0 );
         //  glVertex3f( .8f, 0.05f, 0.0 );  glVertex3f( 1.0, 0.25f, 0.0 ); // Letter X
         //  glVertex3f( 0.8f, .25f, 0.0 );  glVertex3f( 1.0, 0.05f, 0.0 );
         
         glVertex3f( 0, 0, 0 );  glVertex3f( 50.0f, 0, 0 );// X axis
         
         glColor3f( 0.0, 1.0, 0.0 );
         glVertex3f( 0, 0, 0 );  glVertex3f( 0, 50.0f, 0 ); //Y axis
         
         glColor3f( 0.0, 0.0, 1.0 );
         glVertex3f( 0, 0, 0 );  glVertex3f( 0, 0, 50.0f );  // Z axis
         glEnd();
         
         glPopMatrix();
         
         glEnable( GL_LIGHTING );
         */
    }
    
    //draw the grid
    if( draw_grid == 1)
    {
        //disable the light
        glDisable(GL_LIGHTING);
        glLineWidth( 2.0f );
        glColor3f( 1.0f,1.0f,1.0f );
        // glScalef( scale_size_model, scale_size_model, scale_size_model );
        
        glBegin(GL_LINES);
        for(j=-6.0f;j<0.0f;j+=0.5f)
        {
            glVertex3f(j,0.0f,-6.0f);
            glVertex3f(j,0.0f,6.0f);
        }
        for(j=0.5f;j<=6.0f;j+=0.5)
        {
            glVertex3f(j,0.0f,-6.0f);
            glVertex3f(j,0.0f,6.0f);
        }
        
        for(j=-6.0f;j<0.0f;j+=0.5)
        {
            glVertex3f(-6.0f,0.0f,j);
            glVertex3f(6.0f,0.0f,j);
        }
        for(j=0.5f;j<=6.0f;j+=0.5)
        {
            glVertex3f(-6.0f,0.0f,j);
            glVertex3f(6.0f,0.0f,j);
        }
        glEnd();
        glLineWidth( 2.0f );
        glColor3f(0.0f,0.0f,0.0f);
        glBegin(GL_LINES);
        glVertex3f(0,0,0 -6.0f);
        glVertex3f(0,0,6.0f);
        glVertex3f( -6.0f,0,0);
        glVertex3f(6.0f,0,0);
        glEnd();
        glColor3f(r,g,b);
        
        
        glEnable(GL_LIGHTING);
    }
    
    //the user has import a model, then load = 1 and start to draw the model
    if( load == 1 )
    {
        
        
        
        //     printf("center--x:%f,y:%f,z:%f",center[0],center[1],center[2]);
        
        //the user import another model again
        if( reload == 1 )
        {
            
            
            x_dis=y_angle=0.0f;
            x_angle = -30.0f;
            y_dis = -1.0f;
            scale_size=2.0f;
            x_dis_model=y_dis_model=0.0f;
            x_angle_model=y_angle_model=0.0f;
            scale_size_model=1.0f;
            
        }
        
        //draw the bouding box
        if( draw_box == 1)
        {
            // glDisable(GL_LIGHTING);
            glColor3f( 0.5f,0.5f,0.5f );
            glLineWidth( 2.0f );
            glPushMatrix();
            glTranslatef(x_dis_model,y_dis_model,0.0f);
            glRotatef(x_angle_model, 0, 1,0);
            glRotatef(y_angle_model, 1,0,0);
            glScalef(scale_size_model, scale_size_model, scale_size_model);
            
            //draw up surface
            glBegin(GL_LINE_LOOP);
            glVertex3f(v_up[0].x/Vnormal-center[0]/Vnormal,v_up[0].y/Vnormal-center[1]/Vnormal,v_up[0].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_up[1].x/Vnormal-center[0]/Vnormal,v_up[1].y/Vnormal-center[1]/Vnormal,v_up[1].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_up[2].x/Vnormal-center[0]/Vnormal,v_up[2].y/Vnormal-center[1]/Vnormal,v_up[2].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_up[3].x/Vnormal-center[0]/Vnormal,v_up[3].y/Vnormal-center[1]/Vnormal,v_up[3].z/Vnormal-center[2]/Vnormal);
            
            glEnd();
            
            //draw down surface
            glBegin(GL_LINE_LOOP);
            glVertex3f(v_down[0].x/Vnormal-center[0]/Vnormal,v_down[0].y/Vnormal-center[1]/Vnormal,v_down[0].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_down[1].x/Vnormal-center[0]/Vnormal,v_down[1].y/Vnormal-center[1]/Vnormal,v_down[1].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_down[2].x/Vnormal-center[0]/Vnormal,v_down[2].y/Vnormal-center[1]/Vnormal,v_down[2].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_down[3].x/Vnormal-center[0]/Vnormal,v_down[3].y/Vnormal-center[1]/Vnormal,v_down[3].z/Vnormal-center[2]/Vnormal);
            
            glEnd();
            
            //draw the front,left,back,right surface
            glBegin(GL_LINES);
            glVertex3f(v_up[0].x/Vnormal-center[0]/Vnormal,v_up[0].y/Vnormal-center[1]/Vnormal,v_up[0].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_down[0].x/Vnormal-center[0]/Vnormal,v_down[0].y/Vnormal-center[1]/Vnormal,v_down[0].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_up[1].x/Vnormal-center[0]/Vnormal,v_up[1].y/Vnormal-center[1]/Vnormal,v_up[1].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_down[1].x/Vnormal-center[0]/Vnormal,v_down[1].y/Vnormal-center[1]/Vnormal,v_down[1].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_up[2].x/Vnormal-center[0]/Vnormal,v_up[2].y/Vnormal-center[1]/Vnormal,v_up[2].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_down[2].x/Vnormal-center[0]/Vnormal,v_down[2].y/Vnormal-center[1]/Vnormal,v_down[2].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_up[3].x/Vnormal-center[0]/Vnormal,v_up[3].y/Vnormal-center[1]/Vnormal,v_up[3].z/Vnormal-center[2]/Vnormal);
            glVertex3f(v_down[3].x/Vnormal-center[0]/Vnormal,v_down[3].y/Vnormal-center[1]/Vnormal,v_down[3].z/Vnormal-center[2]/Vnormal);
            
            glEnd();
            glPopMatrix();
            glColor3f(r,g,b);
            glLineWidth(1.0f);
            //   glEnable(GL_LIGHTING);
        }
        
        //transform the model
        glPushMatrix();
        glTranslatef(x_dis_model,y_dis_model,0.0f);
        glRotatef(x_angle_model, 0, 1,0);
        glRotatef(y_angle_model, 1,0,0);
        glScalef(scale_size_model, scale_size_model, scale_size_model);
        Draw_model();
        glPopMatrix();
        
        
    }
    
    
    if(show_text == 1){
        
        glDisable( GL_LIGHTING );  //Disable lighting while we render text
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        gluOrtho2D( 0.0, 100.0, 0.0, 100.0  );
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
        glColor3ub( 0, 0, 0 );
        glRasterPos2i( 20, 90);
        
        
        /*  printf( "text: %s\n", text );              */
        
        /*** Render the live character array 'text' ***/
        //   int a = 10;
        stringstream ss;
        stringstream ss2;
        ss << index_face;
        ss2 << index_vertex;
        string temp_str = ss.str();
        string temp_str2 = ss2.str();
        char* string = "face:";
        char* string1 = (char*) temp_str.c_str();
        char* string2 = ",\n";
        char* string3 = "vertex:";
        char* string4 = (char*) temp_str2.c_str();
        
        char * str3 = (char *) malloc(1 + strlen(string)+ 20 );
        
        strcpy(str3, string);
        strcat(str3, string1);
        strcat(str3, string2);
        strcat(str3, string3);
        strcat(str3, string4);
        
        
        _glutBitmapString(GLUT_BITMAP_HELVETICA_18, str3);
        
    }
    
    reload=0;
    glutPostRedisplay();
    glutSwapBuffers();
}

//This function is a callback of glut to control the
//viewport when the size of the window is changed
void Reshape( int x, int y )
{
    int tx, ty, tw, th;
    GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
    glViewport( tx, ty, tw, th );
    xy_aspect= (GLfloat) tw / (GLfloat) th;
    glutPostRedisplay();
}

void control_cb( int control )
{
    


    if ( control == LIGHT0_ENABLED_ID ) {
        if ( light0_enabled ) {
            glEnable( GL_LIGHT0 );
            light0_spinner->enable();
            sb->enable();
            light_group->enable();
            lights_rot->enable();
        }
        else {
            glDisable( GL_LIGHT0 );
            light0_spinner->disable();
            sb->disable();
            light_group->disable();
            lights_rot->disable();

        }
    }
    else if ( control == LIGHT1_ENABLED_ID ) {
        if ( light1_enabled ) {
            glEnable( GL_LIGHT1 );
            light1_spinner->enable();
            sb1->enable();

        }
        else {
            glDisable( GL_LIGHT1 );
            light1_spinner->disable();
            sb1->disable();

        }
    }
    else if ( control == LIGHT0_INTENSITY_ID && light1_mode ==LIGHT0_DIFFUSE) {
        float v[] = {
            temp[0],  temp[1],
            temp[2],  temp[3] };
//        printf("light1_mode: LIGHT0_DIFFUSE ");

        v[0] *= light0_intensity;
        v[1] *= light0_intensity;
        v[2] *= light0_intensity;
        
        glLightfv(GL_LIGHT0, GL_DIFFUSE, v );
        light0_diffuse[0] = v[0];
        light0_diffuse[1] = v[1];
        light0_diffuse[2] = v[2];

    }
    else if ( control == LIGHT0_INTENSITY_ID && light1_mode == LIGHT0_AMBIENT ) {
        float v1[] = {
            temp[0],  temp[1],
            temp[2],  temp[3] };
 //       printf("light1_mode: LIGHT0_ambient ");
  
        v1[0] *= light0_intensity;
        v1[1] *= light0_intensity;
        v1[2] *= light0_intensity;
        
        glLightfv(GL_LIGHT0, GL_AMBIENT, v1 );
        light0_ambient[0] = v1[0];
        light0_ambient[1] = v1[1];
        light0_ambient[2] = v1[2];
        
    }
    else if ( control == LIGHT0_INTENSITY_ID && light1_mode ==LIGHT0_SPECULAR ) {
        float v2[] = {
            temp[0],  temp[1],
            temp[2],  temp[3] };
 //       printf("light1_mode: LIGHT0_ambient ");
        
        v2[0] *= light0_intensity;
        v2[1] *= light0_intensity;
        v2[2] *= light0_intensity;
        
        glLightfv(GL_LIGHT0, GL_SPECULAR, v2 );
        light0_specular[0] = v2[0];
        light0_specular[1] = v2[1];
        light0_specular[2] = v2[2];
    }
    
    
    else if ( control == LIGHT1_INTENSITY_ID ) {
        float v[] = {
            light1_diffuse[0],  light1_diffuse[1],
            light1_diffuse[2],  light1_diffuse[3] };
        
        v[0] *= light1_intensity;
        v[1] *= light1_intensity;
        v[2] *= light1_intensity;
        
        glLightfv(GL_LIGHT1, GL_DIFFUSE, v );
    }
    else if( control == LIGHT_ID )
    {
        light1_mode = 400 + light_group->get_int_val();
        
     //   GLUI_Master.sync_live_all();
        if(light1_mode == LIGHT0_AMBIENT){
            temp[0] = light0_ambient[0];
            temp[1] = light0_ambient[1];
            temp[2] = light0_ambient[2];
            temp[3] = light0_ambient[3];
        }
        if(light1_mode == LIGHT0_DIFFUSE){
            temp[0] = light0_diffuse[0];
            temp[1] = light0_diffuse[1];
            temp[2] = light0_diffuse[2];
            temp[3] = light0_diffuse[3];
        }
        if(light1_mode == LIGHT0_SPECULAR){
            temp[0] = light0_specular[0];
            temp[1] = light0_specular[1];
            temp[2] = light0_specular[2];
            temp[3] = light0_specular[3];
        }
      GLUI_Master.sync_live_all();
    }
}




//This function is a callback of glui, it control the actions
//when you use the controls of the glui
void control(int control)
{
    
    //this struct is for the open file dialog

    
    
    //click on the import botton then ask user to choose a file
    if ( control == IMPORT_ID ){
        nfdchar_t *outPath = NULL;
        
        nfdpathset_t pathSet;
        nfdresult_t result = NFD_OpenDialogMultiple( "m,obj", NULL, &pathSet );
        if ( result == NFD_OKAY )
        {
            size_t i;
            for ( i = 0; i < NFD_PathSet_GetCount(&pathSet); ++i )
            {
                nfdchar_t *path = NFD_PathSet_GetPath(&pathSet, i);
                path1 = path;
                printf("Path %i: %s\n", (int)i, path1 );
            }
            //open the open file dialog
            
            //import the .m file
            import (path1,vert,face1,&v_num,&f_num,edge);
            findedge(vert,v_up,v_down);
            set(vert,v_down);
            findedge(vert,v_up,v_down);
            reload = load = 1;
            
            NFD_PathSet_Free(&pathSet);
        }
        else if ( result == NFD_CANCEL )
        {
            puts("User pressed cancel.");
        }
        else
        {
            printf("Error: %s\n", NFD_GetError() );
        }
    }
    
    
    
    //click on the radio group that chooses display mode
    if( control == DISPLAY_ID )
    {
        //get the current value of the radio button
        display_mode=300+radio_group->get_int_val();
    }
    
    //click on the radio group value that chooses projection mode
    if( control == PROJECTION_ID )
    {
        projection_mode=radio_group1->get_int_val();
    }
    
    if( control == CAMERA_ID )
    {
        camera_mode = 20 + camera_group->get_int_val();
        GLUI_Master.set_glutReshapeFunc( Reshape );

    }
    

    
    
}

//This function is a callback of glut which specifies the transformation
//mode according to which button of the mouse is pressed
void Mouse(int button, int state, int x, int y){
    //a button is pressed
    if (state == GLUT_DOWN)
    {
        mouse_x = x; mouse_y = y;
        //press left button,set mode to rotate
        if (button == GLUT_LEFT_BUTTON)
            transform_mode = TRANSFORM_ROTATE;
        //press middle button,set mode to translate
        else if (button == GLUT_MIDDLE_BUTTON)
            transform_mode = TRANSFORM_TRANSLATE;
        //press right button,set mode to scale
        else if (button == GLUT_RIGHT_BUTTON)
            transform_mode = TRANSFORM_SCALE;
    }
    //no button is pressed
    else if (state == GLUT_UP)
    {
        transform_mode = TRANSFORM_NONE;
    }
}

//This functionwhen is a callback of glut which specifies what to do
//when moving the mouse according to the transformation mode
void Motion(int x, int y)
{
    //    scale_size_model=1.0/Vnormal;
    
    //transform the world coordinate system
    if(object_only == 0)
    {
        //the mode is rotate, do the rotation
        if (transform_mode == TRANSFORM_ROTATE)
        {
            x_angle += (GLfloat) (x - mouse_x)/5.0;
            
            if (x_angle > 180.0)
                x_angle -= 360.0;
            else if (x_angle <-180.0)
                x_angle += 360.0;
            
            mouse_x = x;
            
            y_angle += (GLfloat) (y - mouse_y)/5.0;
            
            if (y_angle > 180.0)
                y_angle -= 360.0;
            else if (y_angle <-180.0)
                y_angle += 360.0;
            
            mouse_y = y;
        }
        //the mode is translate,do the translation
        if (transform_mode == TRANSFORM_TRANSLATE)
        {
            x_dis += (GLfloat) (x - mouse_x)/30.0f;
            
            mouse_x = x;
            
            y_dis -= (GLfloat) (y - mouse_y)/30.0f;
            
            mouse_y = y;
        }
        //the mode is scale,do the scaling
        else if (transform_mode == TRANSFORM_SCALE)
        {
            old_size = scale_size;
            
            scale_size *= (1 + (y - mouse_y)/60.0);
            
            if (scale_size <0)
                scale_size = old_size;
            mouse_y = y;
        }
    }
    if(object_only == 1)
    {
        //the mode is rotate, do the rotation
        if (transform_mode == TRANSFORM_ROTATE)
        {
            x_angle_model += (GLfloat) (x - mouse_x)/5.0;
            
            if (x_angle_model > 180.0)
                x_angle_model -= 360.0;
            else if (x_angle_model <-180.0)
                x_angle_model += 360.0;
            
            mouse_x = x;
            
            y_angle_model += (GLfloat) (y - mouse_y)/5.0;
            
            if (y_angle_model > 180.0)
                y_angle_model -= 360.0;
            else if (y_angle_model <-180.0)
                y_angle_model += 360.0;
            
            mouse_y = y;
        }
        //the mode is translate,do the translation
        if (transform_mode == TRANSFORM_TRANSLATE)
        {
            x_dis_model += (GLfloat) (x - mouse_x)/30.0f;
            
            mouse_x = x;
            
            y_dis_model -= (GLfloat) (y - mouse_y)/30.0f;
            
            mouse_y = y;
        }
        //the mode is scale,do the scaling
        else if (transform_mode == TRANSFORM_SCALE)
        {
            old_size = scale_size_model;
            
            scale_size_model *= (1 + (y - mouse_y)/60.0);
            
            if (scale_size_model <0)
                scale_size_model = old_size;
            mouse_y = y;
        }
    }
    
    
    
    // force the redraw function
    glutPostRedisplay();
}

//This is the main function of the program
int main(int argc, char* argv[])
{
    //initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
    //create a glut window
    glutInitWindowPosition( 60, 50 );
    glutInitWindowSize( 1200, 800 );
    //specify the callbacks
    main_window = glutCreateWindow( " 3D Mesh Viewer" );
    
    glutDisplayFunc( Display );
    GLUI_Master.set_glutReshapeFunc( Reshape );
    glutMouseFunc( Mouse );
    glutMotionFunc( Motion );


    
    /****************************************/
    /*       Set up OpenGL lights           */
    /****************************************/
    

    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
   // glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    
    glEnable(GL_LIGHT0);



    
  //  glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_diffuse);
  //  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient1);
   
    glEnable(GL_LIGHT1);
    glEnable( GL_NORMALIZE );
    glEnable(GL_LIGHTING);
    
    // Create the right side GLUI subwindow
    sub_window = GLUI_Master.create_glui_subwindow( main_window,
                                                   GLUI_SUBWINDOW_LEFT );

    
    //Create the import,Background color and Object Color buttons
    import_panel = new GLUI_Panel( sub_window, "", 1);
    new GLUI_Button( import_panel, "Import", IMPORT_ID, control );
    
    
    GLUI_Rollout *light = new GLUI_Rollout(sub_window,           "Lights", false );
    
    GLUI_Panel *light0 = new GLUI_Rollout( light, "Light 1",false );
    GLUI_Panel *light1 = new GLUI_Rollout( light, "Light 2",false );
    

    
    new GLUI_Checkbox( light0, "Enabled", &light0_enabled,
                      LIGHT0_ENABLED_ID, control_cb );
    light0_spinner =
    new GLUI_Spinner( light0, "Intensity:",
                     &light0_intensity, LIGHT0_INTENSITY_ID,
                     control_cb );
    light0_spinner->set_float_limits( 0.0, 1.0 );
    
    light_group = new GLUI_RadioGroup( light0,0,LIGHT_ID,control_cb);
    new GLUI_RadioButton( light_group," Diffuse");

    
    new GLUI_RadioButton( light_group, "Ambient");
    new GLUI_RadioButton( light_group, "Specular   ");
    light_group->set_int_val(0);

//    printf("main——light1_mode: %d,",light1_mode);



    new GLUI_Column(light_group);
    sb = new GLUI_Scrollbar( light_group, "Red",GLUI_SCROLL_HORIZONTAL,
                            &temp[0],LIGHT0_INTENSITY_ID,control_cb);
    sb->set_float_limits(0,1);


    sb = new GLUI_Scrollbar( light_group, "Green",GLUI_SCROLL_HORIZONTAL,
                            &temp[1],LIGHT0_INTENSITY_ID,control_cb);
    sb->set_float_limits(0,1);
    sb = new GLUI_Scrollbar( light_group, "Blue",GLUI_SCROLL_HORIZONTAL,
                            &temp[2],LIGHT0_INTENSITY_ID,control_cb);
    sb->set_float_limits(0,1);
   

 
    
    
    
    lights_rot = new GLUI_Rotation(light0, "Rotate", lights_rotation );
    lights_rot->set_spin( .82 );
    
    new GLUI_Checkbox( light1, "Enabled", &light1_enabled,
                      LIGHT1_ENABLED_ID, control_cb );
    light1_spinner =
    new GLUI_Spinner( light1, "Intensity:",
                     &light1_intensity, LIGHT1_INTENSITY_ID,
                     control_cb );
    light1_spinner->set_float_limits( 0.0, 1.0 );

    light_group->set_int_val(0);

 

    sb1 = new GLUI_Scrollbar( light1, "Red",GLUI_SCROLL_HORIZONTAL,
                            &light1_diffuse[0],LIGHT1_INTENSITY_ID,control_cb);
    sb1->set_float_limits(0,1);
    sb1 = new GLUI_Scrollbar( light1, "Green",GLUI_SCROLL_HORIZONTAL,
                            &light1_diffuse[1],LIGHT1_INTENSITY_ID,control_cb);
    sb1->set_float_limits(0,1);
    sb1 = new GLUI_Scrollbar( light1, "Blue",GLUI_SCROLL_HORIZONTAL,
                            &light1_diffuse[2],LIGHT1_INTENSITY_ID,control_cb);
    sb1->set_float_limits(0,1);
    

  //  GLUI_Rotation *lights_rot2 = new GLUI_Rotation(light1, "Rotate", lights_rotation2 );
   // lights_rot2->set_spin( .82 );
    
    GLUI_Rollout *camera = new GLUI_Rollout(sub_window,          "Camera", false );
    camera_group = new GLUI_RadioGroup( camera,0,CAMERA_ID,control);
    new GLUI_RadioButton( camera_group,"3D");
    new GLUI_RadioButton( camera_group, "YZ");
    new GLUI_RadioButton( camera_group, "ZX");
    new GLUI_RadioButton( camera_group, "XY");
    camera_group->set_int_val(0);
    
    
    
    
    
    //create a panel that include 4 radio button to choose display mode
    //new GLUI_StaticText(sub_window," ");
    // display_panel = new GLUI_Panel( sub_window, "Display Model", 1);
    GLUI_Rollout *display_panel = new GLUI_Rollout(sub_window,   "Display Model", false );
    radio_group = new GLUI_RadioGroup( display_panel,0,DISPLAY_ID,control);
    new GLUI_RadioButton( radio_group,"Points");
    new GLUI_RadioButton( radio_group, "Wireframe");
    new GLUI_RadioButton( radio_group, "Flat shading");
    new GLUI_RadioButton( radio_group, "Smooth shading");
    radio_group->set_int_val(3);
    
    //create a panel that include 2 radio button to choose projection mode
    //new GLUI_StaticText(sub_window," ");
    
    
    GLUI_Rollout *projection_panel = new GLUI_Rollout(sub_window,"Projection Model", false );
    radio_group1 = new GLUI_RadioGroup( projection_panel,0,PROJECTION_ID,control);
    new GLUI_RadioButton( radio_group1,"Orthogonal");
    new GLUI_RadioButton( radio_group1, "Perspective");
    radio_group1->set_int_val(1);
    
    //create 3 checkboxs the decide draw the axe,grid,bounding box or not
    cor_system = new GLUI_Panel( sub_window, "", 1);
    new GLUI_Checkbox( cor_system, "Draw axe", &draw_axe );
    new GLUI_Checkbox( cor_system, "Draw grid", &draw_grid );
    new GLUI_Checkbox( cor_system, "Draw bounding box", &draw_box );
    new GLUI_Checkbox( cor_system, "Transform model only", &object_only );
    new GLUI_Checkbox( cor_system, "Text", &show_text );
    
    
    new GLUI_StaticText(sub_window," ");
    new GLUI_StaticText(sub_window,"Mouse control:");
    new GLUI_StaticText(sub_window,"Left Button : Rotation");
    new GLUI_StaticText(sub_window,"Middle Button : Translation");
    new GLUI_StaticText(sub_window,"Right Button : Zoom in/out");
    
    //display a instruction how to use the mouse
    new GLUI_StaticText(sub_window," ");
    new GLUI_StaticText(sub_window," ");
    new GLUI_StaticText(sub_window," ");
    
    //create a exit button
    new GLUI_Button( sub_window, "Exit", 0,(GLUI_Update_CB)exit );
    
    
    
    //link windows to GLUI
    sub_window->set_main_gfx_window( main_window );
    
    //creat the bottom side GLUI subwindow
    sub_window1 = GLUI_Master.create_glui_subwindow( main_window,
                                                    GLUI_SUBWINDOW_BOTTOM );
    //put a static text to specify the author and date
    new GLUI_StaticText(sub_window1,
                        "                                             Mesh Viewer, WEI SICONG, 15/10/2016 NTU_DMT Advanced Computer Graphics  ------- He Ying" );
    //link windows to GLUI
    sub_window1->set_main_gfx_window( main_window );
    
    //regular GLUT main loop
    glutMainLoop();
    
    return 0;
}