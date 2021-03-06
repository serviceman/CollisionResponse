// CollisionResponse.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>
#include <windows.h>	// for timeGetTime()
#include <mmsystem.h>	// ditto
#include <iostream>		// I/O
#include <glut.h>// GLUT
#include <time.h>

#include <vector>
#include <map>

#include "Body.h"
#include "Plane.h"


#ifndef PI
#define PI 3.14159265
#endif

//#include "Vector.h"

#define NUMCUBES 7


/**********MATERIALS*********/
GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0}; //lol
GLfloat left_light_position[] = {-7,5,-7, 1.0};
GLfloat right_light_position[] = {7,5,7, 1.0};

GLfloat ruby_ambient[]  = {0.1745,	 0.01175,	 0.01175,	 0.55},
        ruby_diffuse[]  = {0.61424,	 0.04136,	 0.04136,	 0.55},
        ruby_specular[] = {0.727811,	 0.626959,	 0.626959,	 0.55},
        ruby_shininess  = 76.8;

GLfloat gold_ambient[]  = {0.24725,	 0.1995,	 0.0745,	 1},
        gold_diffuse[]  = {0.75164,	 0.60648,	 0.22648,	 1},
        gold_specular[] = {0.628281,	 0.555802,	 0.366065,	 1},
        //gold_specular[] = {0.0,	 0.0,	 0.0,	 1},
        gold_shininess  = 51.2;

GLfloat jade_ambient[]  = {0.135,	 0.2225,	 0.1575,	 0.95},
        //jade_ambient[]  = {0,	 1,	 0,	 0.95},
        jade_diffuse[]  = {0.54,	 0.89,	 0.63,	 0.95},
        jade_specular[] = {0.316228,	 0.316228,	 0.316228,	 0.95},
        jade_shininess  = 12.8;

GLfloat grey_ambient[] = {0.01745, 0.01745, 0.01745},
        grey_diffuse[] = {0.161424, 0.161424, 0.161424},
        grey_specular[] = {0.727811, 0.727811, 0.727811},
        grey_shininess = 76.8;

GLfloat plain_ambient[] = {0.01745, 0.01745, 0.01745, 0},
        plain_diffuse[] = {1, 1, 1, 1},
        plain_specular[] = {0.727811, 0.727811, 0.727811},
        plain_shininess = 76.8;
/*****END OF MATERIALS****/

enum {jade,ruby,gold,grey,plain};


//camera views
float zoom = 30.0f;
float rotx = 30.0;
float roty = -220.0f;

//mouse control
float tx = 0;
float ty = 0;
int lastx= 0;
int lasty= 0;
unsigned char Buttons[3] = {0};

//time indicator
int frames=0;

void setupScene();
void updateScene();
void renderScene();
void exitScene();
void keypress(unsigned char key, int x, int y);
void setViewport(int width, int height);

bool		wireframe=true;
int         windowId;
DWORD		lastTickCount;

float cameraRotation;
float cameraTarget;


std::vector<Vector> colliding_points;
Vector mean_coll_point;
Vector yan;

std::vector<Force> forces;
std::vector<Body> collection;

bool collision = false;


//Plane pl( Vector(-2,3.5,-2), Vector(-2,1,10), Vector(5,-3,10), Vector(5,-0.5,-2) ); //from mathematica paper
Plane pl(Vector(-5,0.2,-5), Vector(-5,0.2,5), Vector(5,0.2,5), Vector(5,0.2,-5) );


void cameraTransform()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0,0,-zoom);
    glTranslatef(tx,ty,0);
    glRotatef(rotx,1,0,0);
    glRotatef(roty-170,0,1,0);

}


void print_bitmap_string(void* font, char* s)
{
   if (s && strlen(s)) {
      while (*s) {
         glutBitmapCharacter(font, *s);
         s++;
      }
   }
}

void drawAxes()
{
    glLineWidth(2);

    glColor3f(255,0,0); //RED: x
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(5,0,0);
    glEnd();

    glColor3f(0,0,255); //BLUE: z
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(0,0,5);
    glEnd();

    glColor3f(0,255,0); //GREEN: y
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(0,5,0);
    glEnd();

    glLineWidth(1);
}

void drawGrid()
{
    glColor3f(0,0,0); //BLACK: grid
    glBegin(GL_LINES);
    for(int i=-10;i<=10;++i)
    {
        glVertex3f(i,0,-10);
        glVertex3f(i,0,10);

        glVertex3f(10,0,i);
        glVertex3f(-10,0,i);
    }
    glEnd();
}

void fog()
{
    glEnable (GL_FOG);

    glFogi (GL_FOG_MODE, GL_EXP2);

    GLfloat fogCol[4] = {0.5, 0.5, 0.5, 1.0};
    glFogfv (GL_FOG_COLOR, fogCol);

    glFogf (GL_FOG_DENSITY, 0.01);

    glHint (GL_FOG_HINT, GL_NICEST);
}

void setMaterial(int material)
{
    switch (material) {
        case jade:
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, jade_ambient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, jade_diffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, jade_specular);
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, jade_shininess);
            break;
        case ruby:
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ruby_ambient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ruby_diffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ruby_specular);
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, ruby_shininess);
            break;
    }
}

void cubeWorld()
{
    //TUTORIAL texture (not a skybox?)

    //NOTE: never mix textures with materials
    // Store the current matrix
    glPushMatrix();

    // Reset and transform the matrix.
    //glLoadIdentity(); //mess if uncomment (because it loads the identity without the camera settings)
    //cameraTransform(); //not working :(

    // Enable/Disable features
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);
    //glDisable(GL_DEPTH_TEST); //there is no occlusion?!
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    // Just in case we set all vertices to white.
    glColor4f(1,1,1,1);

 //   // Render the front quad
 //   glBindTexture(GL_TEXTURE_2D, texture[2]);
 //   glBegin(GL_QUADS);
    ////glNormal3f(-1, 0, 0); //looks unnecessary... INVESTIGATE.
 //       glTexCoord2f(0, 0); glVertex3f(  50, -0.1f, -50 );
 //       glTexCoord2f(1, 0); glVertex3f( -50, -0.1f, -50 );
 //       glTexCoord2f(1, 1); glVertex3f( -50,  50.1f, -50 );
 //       glTexCoord2f(0, 1); glVertex3f(  50,  50.1f, -50 );
 //   glEnd();
 //
 //   // Render the left quad
 //   glBindTexture(GL_TEXTURE_2D, texture[2]); //  GL knows that I enabled the textures, and it just uses the last binded one
 //   glBegin(GL_QUADS);
    ////glNormal3f(1, 0, 0); //looks unnecessary... INVESTIGATE.
 //       glTexCoord2f(0, 0); glVertex3f(  50, -0.1f,  50 );
 //       glTexCoord2f(1, 0); glVertex3f(  50, -0.1f, -50 );
 //       glTexCoord2f(1, 1); glVertex3f(  50,  50.1f, -50 );
 //       glTexCoord2f(0, 1); glVertex3f(  50,  50.1f,  50 );
 //   glEnd();
 //
 //   // Render the back quad
 //   glBindTexture(GL_TEXTURE_2D, texture[2]);
 //   glBegin(GL_QUADS);
    ////glNormal3f(0, 0, -1); //looks unnecessary... INVESTIGATE.
 //       glTexCoord2f(0, 0); glVertex3f( -50, -0.1f,  50 );
 //       glTexCoord2f(1, 0); glVertex3f(  50, -0.1f,  50 );
 //       glTexCoord2f(1, 1); glVertex3f(  50,  50.1f,  50 );
 //       glTexCoord2f(0, 1); glVertex3f( -50,  50.1f,  50 );
 //
 //   glEnd();
 //
 //   // Render the right quad
 //   glBindTexture(GL_TEXTURE_2D, texture[2]);
 //   glBegin(GL_QUADS);
    ////glNormal3f(0, 0, 1); //looks unnecessary... INVESTIGATE.
 //       glTexCoord2f(0, 0); glVertex3f( -50, -0.1f, -50 );
 //       glTexCoord2f(1, 0); glVertex3f( -50, -0.1f,  50 );
 //       glTexCoord2f(1, 1); glVertex3f( -50,  50.1f,  50 );
 //       glTexCoord2f(0, 1); glVertex3f( -50,  50.1f, -50 );
 //   glEnd();
 //
 //   // Render the top quad
 //   glBindTexture(GL_TEXTURE_2D, texture[1]);
 //   glBegin(GL_QUADS);
    ////glNormal3f(0, 1, 0); //looks unnecessary... INVESTIGATE.
 //       glTexCoord2f(0, 1); glVertex3f( -50, 50.1f, -50 );
 //       glTexCoord2f(0, 0); glVertex3f( -50, 50.1f,  50 );
 //       glTexCoord2f(1, 0); glVertex3f(  50, 50.1f,  50 );
 //       glTexCoord2f(1, 1); glVertex3f(  50, 50.1f, -50 );
 //   glEnd();

    // Render the bottom quad
    //glBindTexture(GL_TEXTURE_2D, texture[0]);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0); //pullula di normali... proprio...
        glTexCoord2f(0, 0); glVertex3f( -50, -0.1f, -50 );
        glTexCoord2f(0, 1); glVertex3f( -50, -0.1f,  50 );
        glTexCoord2f(1, 1); glVertex3f(  50, -0.1f,  50 );
        glTexCoord2f(1, 0); glVertex3f(  50, -0.1f, -50 );
    glEnd();


    glEnable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    // Restore enable bits and matrix
    glPopAttrib();
    glPopMatrix();
}

void writeOnScreen(char *message,float x,float y,float window_width,float window_heigth)
{
    GLvoid *font_style = GLUT_BITMAP_TIMES_ROMAN_24;

    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();

    glLoadIdentity ();

    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();

    glLoadIdentity ();
    gluOrtho2D (0.0, window_width, 0.0, window_heigth);
    glRasterPos2f(x, y);
    print_bitmap_string( font_style, message );

    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();

    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();
}

void baraff(Body &cub, Vector collidingpoint, Vector n)
{


    Vector arm = collidingpoint-cub.position;
    Vector padot = cub.velocity + (cub.angularVelocity.crossProduct(arm)) ;
    Vector pbdot(0,0,0);//plane

    double epsilon = 0.5;
    //double epsilon = 1;//no kinetic energy is lost
    double vrel = n.dotProduct(padot);
    double numerator = -1*(1+epsilon) * vrel;

    double t1 = cub.massR;
    double t2 = 0;
    double t3 = n.dotProduct((cub.worldInertialTensor*(arm.crossProduct(n))).crossProduct(arm));
    double t4 = 0;

    //calculation
    float j = numerator / (t1+t2+t3+t4);
    Vector pul = n*j;

    pul*=0.2; // scale that hacks it.. otherwise it just explodes

    float vel = cub.velocity.length();
    if(vel < 0.11)  //resting contact
        pul = Vector (0,0,0);

    //application && update
    cub.velocity += pul*cub.massR;
    Vector temp = cub.worldInertialTensor * arm.crossProduct(pul);
    cub.angularVelocity += temp;

}


void NPcollisionCheck()
{
    //reset collision flag & book
    colliding_points.clear();
    collision = false;


    for(int j=0;j<collection.size();j++)
            //use of AABB for plane collision
            if(collection.at(j).box.min.y < pl.origin.y){
                collection.at(j).setCollision(true);
                collision = true;

                //collision checking (which vertices)
                for( int i = 0; i < collection.at(j).worldVertices.size(); i++){
                    //std::cout << cube1.worldVertices.at(i) << std::endl;
                    std::cout << "vertex " << i << ": " << pl.getDistance(collection.at(j).worldVertices.at(i)) << std::endl;
                    float dist = pl.getDistance(collection.at(j).worldVertices.at(i));
                    if( dist < 0 ){
                        colliding_points.push_back(collection.at(j).worldVertices.at(i));
                        //raise COM to the distance
                        collection.at(j).position += pl.getNormal()*dist*-1;//the distance is negative
                        //std::cout << pl.getNormal() << " " << pl.getDistance(collection.at(j).worldVertices.at(i)) << std::endl;
                    }
                }
            }
            else {
                collection.at(j).setCollision(false);
                collision = false;
            }

        if(collision){


                //for(int j=0;j<collection.size();j++){
                //	for( int i = 0; i < collection.at(j).worldVertices.size(); i++){
                //		//std::cout << cube1.worldVertices.at(i) << std::endl;
                //		//std::cout << "vertex " << i << ": " << pl.getDistance(collection.at(j).worldVertices.at(i)) << std::endl;
                //		if( pl.getDistance(collection.at(j).worldVertices.at(i)) < 0 ){
                //			colliding_points.push_back(collection.at(j).worldVertices.at(i));
                //		}
                //	}
                //}

            //bookkeeping
                mean_coll_point = Vector(0,0,0);
                float maxDist = 1;
                for( int i=0; i < colliding_points.size(); i++){
                    std::cout << "vertices colliding: " << colliding_points.at(i) << " ; indeed its distance from the plane is " << pl.getDistance(colliding_points.at(i)) << std::endl;

                    if( maxDist > pl.getDistance(colliding_points.at(i)) ){
                        maxDist = pl.getDistance(colliding_points.at(i));
                        mean_coll_point = colliding_points.at(i);
                    }

                }

                std::cout << "resulting (MINd) vertex colliding: " << mean_coll_point << std::endl;

            }

        //another normal (this time using formula)
        Vector close;

        close = pl.getClosestPoint(mean_coll_point);
        yan.x = (mean_coll_point.x - close.x ) / mean_coll_point.distance( close );
        yan.y = (mean_coll_point.y - close.y ) / mean_coll_point.distance( close );
        yan.z = (mean_coll_point.z - close.z ) / mean_coll_point.distance( close );

}


void renderScene(){

    // Clear framebuffer & depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glClearColor(1.0,1.0,1.0,1.0); //I actually need to call it only once, so if i put it in setupscene is ok
    cameraTransform();

    glDisable(GL_LIGHTING);

    drawAxes();

    drawGrid();

    //fog();

    //cubeWorld();


    glEnable(GL_LIGHTING); //Enable global lighting

    ////WITH = All the light will NOT move with the camera (light fixed even if i rotate)
    ////Because i tell at every frame to fix the light at those 2 points
    glLightfv(GL_LIGHT0, GL_POSITION, left_light_position);
    glLightfv(GL_LIGHT1, GL_POSITION, right_light_position);

    for(int i=0;i<collection.size();i++)
        collection.at(i).draw(Vector(0,0,0.6f));

    pl.draw(collision?Vector(0.7f,0.2f,0.1f):Vector(0.4f,0.3f,0));

    if(collision){
        //contact point
        glPointSize(7);
        glColor3f(1.0f,0,0);
        glBegin(GL_POINTS);
            glVertex3f(mean_coll_point.x,mean_coll_point.y,mean_coll_point.z);
        glEnd();

        //normal
        glColor3f(1.0f,0,1.0f);
        glBegin(GL_LINES);
            glVertex3d(mean_coll_point.x,mean_coll_point.y,mean_coll_point.z);
            glVertex3d(mean_coll_point.x+yan.x, mean_coll_point.y+yan.y, mean_coll_point.z+yan.z);
        glEnd();

        //marking
        //for( int i = 0; i < cube1.worldVertices.size(); i++){ //closest point marking
        //	glBegin(GL_LINES);
        //		glVertex3d(cube1.worldVertices.at(i).x,cube1.worldVertices.at(i).y,cube1.worldVertices.at(i).z);
        //		glVertex3d(pl.getClosestPoint(cube1.worldVertices.at(i)).x,pl.getClosestPoint(cube1.worldVertices.at(i)).y,pl.getClosestPoint(cube1.worldVertices.at(i)).z);
        //	glEnd();
        //}
        //for( int i = 0; i < cube1.worldVertices.size(); i++){ //origin to vertices
        //	glBegin(GL_LINES);
        //	glVertex3d(pl.origin.x,pl.origin.y,pl.origin.z);
        //	glVertex3d(cube1.worldVertices.at(i).x,cube1.worldVertices.at(i).y,cube1.worldVertices.at(i).z);
        //	glEnd();
        //}

        //projection
        /*glBegin(GL_LINES);
                glVertex3d(mean_coll_point.x, mean_coll_point.y, mean_coll_point.z);
                glVertex3d(pl.getClosestPoint(mean_coll_point).x, pl.getClosestPoint(mean_coll_point).y, pl.getClosestPoint(mean_coll_point).z);
        glEnd();*/

        //normal (of the plane)
        /*glBegin(GL_LINES);
            glVertex3f(mean_coll_point.x, mean_coll_point.y, mean_coll_point.z);
            glVertex3f(mean_coll_point.x+pl.normal.x, mean_coll_point.y+pl.normal.y, mean_coll_point.z+pl.normal.z);
        glEnd();*/
    }


    glutSwapBuffers();

}


void updateScene()
{
        std::cout << "----------------------------" <<std::endl;
        frames ++ ;
        // Wait until at least 16ms passed since start of last frame
        // Effectively caps framerate at ~60fps
        while(timeGetTime()-lastTickCount<16);
        lastTickCount=timeGetTime();


        for(int i=0;i<collection.size();i++)
            collection.at(i).update(forces);


        NPcollisionCheck();


        if(collision)
        {
            baraff(collection.at(0), mean_coll_point, yan*-1);
            //collection.at(0).velocity += Vector(0,2,0); //test
        }


        // Draw the next frame
        glutPostRedisplay();

}

void keypress(unsigned char key, int x, int y){

    // Test if user pressed ESCAPE (ascii 27)
    // If so, exit the program
    if(key==27){
        exitScene();
    }

    // 'W' key toggles wireframe mode on & off
    if(key == 'w' || key == 'W'){
        wireframe=!wireframe;
        if(wireframe){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if(key=='s'){
        collection.at(0).stop();
    }

    if(key=='r'){
        collection.at(0).reset();
    }

    if(key=='z'){
        forces.clear();
        collection.at(0).velocity.zero();
    }

    if(key=='x'){
        forces.push_back( Force(Vector(0.0,-0.02,0.0)) );
    }

    if(key=='8'){
        pl.origin.y+=0.1;
        pl.V1.y+=0.1;
        pl.V2.y+=0.1;
        pl.V3.y+=0.1;
        pl.V4.y+=0.1;
    }

    if(key=='2'){
        pl.origin.y-=0.1;
        pl.V1.y-=0.1;
        pl.V2.y-=0.1;
        pl.V3.y-=0.1;
        pl.V4.y-=0.1;
    }

    if(key=='u'){
        updateScene();
    }




}

void keyreleased(unsigned char key, int x, int y){

}

//-------------------------------------------------------------------------------
//
void Motion(int x,int y)
{
    int diffx=x-lastx;
    int diffy=y-lasty;
    lastx=x;
    lasty=y;

    if( Buttons[2] )
    {
        zoom -= (float) 0.15f * diffy;
    }
    else if( Buttons[0] )
    {
        rotx += (float) 0.5f * diffy;
        roty += (float) 0.5f * diffx;
    }
    else if( Buttons[1] )
    {
        tx += (float) 0.05f * diffx;
        ty -= (float) 0.05f * diffy;
    }

}

//-------------------------------------------------------------------------------
//
void Mouse(int b,int s,int x,int y)
{
    lastx=x;
    lasty=y;
    switch(b)
    {
    case GLUT_LEFT_BUTTON:
        Buttons[0] = ((GLUT_DOWN==s)?1:0);
        break;
    case GLUT_MIDDLE_BUTTON:
        Buttons[1] = ((GLUT_DOWN==s)?1:0);
        break;
    case GLUT_RIGHT_BUTTON:
        Buttons[2] = ((GLUT_DOWN==s)?1:0);
        break;
    default:
        break;
    }

}

void setupScene(){

    std::cout<<"Initializing scene..."<<std::endl;

    std::cout.precision(5);

    //Set up Lighting Stuff
    glLightfv(GL_LIGHT0, GL_POSITION, left_light_position);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT1, GL_POSITION, right_light_position);
    glLightfv(GL_LIGHT1, GL_SPECULAR, white_light);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, white_light);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);


    glEnable(GL_COLOR_MATERIAL); //CODIIIIO


    Body cube1(Vector(0,7,0));
    Body cube2(Vector(-2,5,0));

    //forces.push_back(Vector(-0.001,0.0,0.0));//wind
    forces.push_back( Force(Vector(0.0,-0.015,0.0)) );//not
    //forces.push_back(Vector(0.0,-0.09,0.0));//ok 0.15; gravity force
    collection.push_back(cube1);
    //collection.at(0).velocity = Vector ( 0,-2,0);
    //collection.push_back(cube2);

}

void exitScene()
{

    std::cout<<"Exiting scene..."<<std::endl;

    // Close window
    glutDestroyWindow(windowId);

    // Free any allocated memory

    // Exit program
    std::exit(0);
}

void setViewport(int width, int height) {

    // Work out window ratio, avoid divide-by-zero
    if(height==0) height=1;
    float ratio = float(width)/float(height);

    // Reset projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Fill screen with viewport
    glViewport(0, 0, width, height);

    // Set a 45 degree perspective
    gluPerspective(45, ratio, .1, 1000);

}

int main(int argc, char *argv[]){


    srand ( time(NULL) );


    // Initialise OpenGL
    glutInit(&argc, argv);

    // Set window position, size & create window
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(550,50);
    glutInitWindowSize(640,480);
    windowId = glutCreateWindow("IET Physics Lab 5: Rigid Body Collision Response");

    // Set GLUT callback functions
    glutReshapeFunc(setViewport);
    glutDisplayFunc(renderScene);
    glutIdleFunc(updateScene);
    glutKeyboardFunc(keypress);
    glutKeyboardUpFunc(keyreleased);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);


    glClearColor(0.5,0.5,0.5,1.0); //I actually need to call it only once, so if i put it in setupscene is ok
    // Setup OpenGL state & scene resources (models, textures etc)
    setupScene();

    // Show window & start update loop
    glutMainLoop();

    return 0;

}
