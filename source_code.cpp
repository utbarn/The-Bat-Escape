using namespace std;
#include<iostream>
#include<math.h>
#include <stdio.h>
#include<stdlib.h>
#include "ifs.h"
#include "ifs.c"
#include <GL/gl.h>
#include <GL/glut.h>
#include <termios.h>
#include<limits.h>
#include <list>
#define PI 3.14159265
#define WIDTH 1000
#define HEIGHT 640
#include <string.h>
#include<ostream>
#include <sstream>


float Time=100;
int center_m_lag=25;
int left_m_lag=25;
int right_m_lag=25;
float change_direction_factor=0.05;
float rotate_factor=0.01;
int Win_id;  //window id
bool game_end=false; //true when game ends

//------------------------------Global data--------------------------------------------
IFS_DATA* batwing;

float centerX;
float  centerY;
float  centerZ;
float  eyeX;
float  eyeY;
float  eyeZ;
float  upX,upY,upZ;
float fov=45,aspectratio=(float)WIDTH/(float)HEIGHT,near=1,far=40;

float rotated=0;	      // angle by which batwing is rotated
float direcX,direcY,direcZ; //direction in which the batwing moves
bool move;
float directhetha=0;	//dirction of batwing (angle from the z-axis)

//------------------------------------code to display messages-------------------------------------------------
int width,height;
unsigned char * data;
void *font = GLUT_STROKE_ROMAN;
void *fonts[] = {GLUT_STROKE_ROMAN, GLUT_STROKE_MONO_ROMAN};
const char *message;

void selectMessage(int msg)
{
	std::ostringstream ss;
	ss << int(Time);
	std::string s(ss.str());
	const char *remtime=s.c_str();
  	switch (msg) 
  	{
  		case 1:
 			message = remtime;
    			break;
  		case 2:
    			message = "Time Out!!!"; 
    			break;
    		case 3:
   	 		message = "Bat Wrecked!!!";
    			break;
    		case 4:
    			message = "You Win!";
    			break;
  	}
}



//------------------------------Vector4 class------------------------------------------

class Vector4
{
	float x,y,z,a;
	public:
	Vector4()
	{
	}
	Vector4(float xx,float yy,float zz,float aa)
	{
	x=xx,y=yy,z=zz,a=aa;
	}
	float get_x()
	{
		return x;
	}
	float get_y()
	{
		return y;
	}
	float get_z()
	{
		return z;
	}
	float get_a()
	{
		return a;
	}
	Vector4 operator+(Vector4 v)
	{
		Vector4 res;
		res.x=x+v.x;
		res.y=y+v.y;
		res.z=z+v.z;
		res.a=a+v.a;
		return res;
	}
	Vector4 operator-(Vector4 v)
	{
		Vector4 res;
		res.x=x-v.x;
		res.y=y-v.y;
		res.z=z-v.z;
		res.a=a-v.a;
		return res;
	}
	void homogenize()
	{
	x=x/a;
	y=y/a;
	z=z/a;
	a=a/a;
	}
	void normalize()
	{
		float f=sqrt(x*x+y*y+z*z);
		x=x/f;
		y=y/f;
		z=z/f;
	}
	void printit()
	{
		cout<<x<<":"<<y<<":"<<z<<":"<<a<<"\n";
	}
};
//------------------------------End of Vector4 class-----------------------------------

//------------------------------Matrix4 class------------------------------------------
class Matrix4
{
	float m[4][4];
	public:
	Matrix4()
	{
	}
	Matrix4(float mm[4][4])
	{
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				m[i][j]=mm[i][j];	
	}
	Matrix4 operator+(Matrix4 M)
	{
		Matrix4 res;
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				res.m[i][j]=m[i][j]+M.m[i][j];
		return res;
	}
	Matrix4 operator-(Matrix4 M)
	{
		Matrix4 res;
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				res.m[i][j]=m[i][j]-M.m[i][j];
		return res;
	}
	Matrix4 operator*(int k)
	{
		Matrix4 res;
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				res.m[i][j]=m[i][j]*k;
		return res;
	}
	Vector4 operator*(Vector4 v)
	{
		float a[4];
		float v1[4];
		v1[0]=v.get_x();
		v1[1]=v.get_y();
		v1[2]=v.get_z();
		v1[3]=v.get_a();
	
		for(int i=0;i<4;i++)
			a[i]=0;
		
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				a[i]+=m[i][j]*v1[j];
		Vector4 v2(a[0],a[1],a[2],a[3]);
		return v2;
	}
	Matrix4 operator*(Matrix4 M)
	{
		Matrix4 res;
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				{
					res.m[i][j]=0;
					for(int k=0;k<4;k++)
						res.m[i][j]+=(m[i][k] * M.m[k][j]);
				}
		return res;
	}

};
//------------------------------End of Matrix4 class-----------------------------------


//--------------------------------Polygon class-------------------------------------------------------

class polygon
{	public:
	float v1[3],v2[3],v3[3],v4[3];
	
	polygon(float a1,float b1,float c1,float a2,float b2,float c2,float a3,float b3,float c3,float a4,float b4,float c4)
	{
		v1[0]=a1;
		v1[1]=b1;
		v1[2]=c1;
		v2[0]=a2;
		v2[1]=b2;
		v2[2]=c2;
		v3[0]=a3;
		v3[1]=b3;
		v3[2]=c3;
		v4[0]=a4;
		v4[1]=b4;
		v4[2]=c4;
	}
	
};

//------------------------------End of Polygon class-----------------------------------

//------------------------------Some supplementary function required------------------- 

//--------------------------------function to return the square-----------------------------------------------------
float sqr(float a)
{
return a*a;
}
//----------------------------rotation about x axis------------------------------------
Vector4 RotateX(Vector4 v,float param)
{
	float m[4][4]={{1,0,0,0},{0,cos(param * PI / 180.0),-sin(param * PI / 180.0),0},{0,sin(param*PI/180.0), cos(param * PI / 180.0),0},{0,0,0,1}};
	Matrix4 M(m);
	return M*v;
}
//----------------------------rotation about y axis------------------------------------
Vector4 RotateY(Vector4 v,float param)
{
	float m[4][4]={{cos(param*PI/180.0),0,sin(param*PI/180.0),0},{0,1,0,0},{-sin(param* PI/180.0),0,cos(param*PI/180.0) ,0},{0,0,0,1}};
	Matrix4 M(m);
	return M*v;
}
//----------------------------rotation about z axis------------------------------------
Vector4 RotateZ(Vector4 v,float param)
{
	float m[4][4]={{cos(param*PI/180.0),-sin(param*PI/180.0),0,0},{sin(param*PI/180.0),cos(param*PI/180.0),0,0},{0,0,1,0},{0,0,0,1}};
	Matrix4 M(m);
	return M*v;
}

//---------------------function to find cross product-----------------------------
Vector4 CrossProduct(Vector4 v1,Vector4 v2/*float x1, float y1, float z1, float x2, float y2, float z2,*/)
{
 float res[3];
res[0] = v1.get_y()*v2.get_z() - v2.get_y()*v1.get_z();
res[1] = v2.get_x()*v1.get_z() - v1.get_x()*v2.get_z();
res[2] = v1.get_x()*v2.get_y() - v2.get_x()*v1.get_y();
Vector4 v(res[0],res[1],res[2],1);
return v;
}
//-----------------------function to rotate about arbitrary axis------------------------------

void rotate(float a,float b,float c,float u,float v,float w,float x,float y, float z,float theta,float &x1,float &y1,float &z1)
{
	float L=sqrt(u*u+v*v+w*w);
	x1=(a*(v*v +w*w)-u*(b*v+c*w-u*x-v*y-w*z))*(1-cos(theta))+L*x*cos(theta)+sqrt(L)*(-c*v+b*w-w*y+v*z)*sin(theta)/L; 
	y1=(b*(u*u+w*w)-v*(a*u+c*w-u*x-v*y-w*z))*(1-cos(theta))+L*y*cos(theta)+sqrt(L)*(c*u-a*w+w*x-u*z)*sin(theta)/L;
	z1=(c*(u*u+v*v)-w*(a*u+b*v-u*x-v*y-w*z))*(1-cos(theta))+L*z*cos(theta)+sqrt(L)*(-b*u+a*v-v*x+u*y)*sin(theta)/L;
}

//-----------------------function to return an array from given vector------------------------------
float* vectortoarray(Vector4 v)
{
float a[3];
a[0]=v.get_x();
a[1]=v.get_y();
a[2]=v.get_z();
return a;
}

//--------------------function to get normal to a traingular plane---------------------
Vector4 get_normal(IFS_DATA* ifs_data,int i)
{
	
	float a1[3],a2[3];
	a1[0]=ifs_data->triangles[i].a->x - ifs_data->triangles[i].c->x;
	a1[1]=ifs_data->triangles[i].a->y - ifs_data->triangles[i].c->y;
	a1[2]=ifs_data->triangles[i].a->z - ifs_data->triangles[i].c->z;
	
	a2[0]=ifs_data->triangles[i].b->x - ifs_data->triangles[i].c->x;
	a2[1]=ifs_data->triangles[i].b->y - ifs_data->triangles[i].c->y;
	a2[2]=ifs_data->triangles[i].b->z - ifs_data->triangles[i].c->z;
	
	Vector4 v1(a1[0],a1[1],a1[2],1);
	Vector4 v2(a2[0],a2[1],a2[2],1);

	Vector4 v3=CrossProduct(v1,v2);
	v3.normalize();
	return v3;
}

//------------------------------end of supplementary functions------------------------------------------

void display();
void draw_scene();


void rotateit(float r) //to rotate the batwing
{
	float x1,y1,z1,x2,y2,z2;
	
   
	x1=((batwing->triangles[556].a->x+batwing->triangles[151].a->x)/2);
	y1=((batwing->triangles[556].a->y+batwing->triangles[151].a->y)/2);
	z1=((batwing->triangles[556].a->z+batwing->triangles[151].a->z)/2);
	for (int i=0; i<batwing->numVertices; ++i) 
	{
		rotate(x1,y1,z1,direcX,direcY,direcZ,batwing->vertices[i].x,batwing->vertices[i].y,batwing->vertices[i].z,r,x2,y2,z2);
		batwing->vertices[i].x=x2;
		batwing->vertices[i].y=y2;
		batwing->vertices[i].z=z2;
	}
}

void changedirection(float r) //to change the direction of batwing
{
	float x1,y1,z1,x2,y2,z2;
	x1=batwing->triangles[841].a->x;
	y1=batwing->triangles[841].a->y;
	z1=batwing->triangles[841].a->z;
	for (int i=0; i<batwing->numVertices; ++i) 
	{
		rotate(x1,y1,z1,0,1,0,batwing->vertices[i].x,batwing->vertices[i].y,batwing->vertices[i].z,r,x2,y2,z2);
		batwing->vertices[i].x=x2;
		batwing->vertices[i].y=y2;
		batwing->vertices[i].z=z2;
	}
			
}

//---------------------function for handling key press events--------------------------
template<class TYPE>
void Controller (TYPE key, int x, int y) 
{
	if(game_end)
		return;
	else if(key=='a')
	{	
		if(rotated<1.5)
		{
				rotateit(0.15);
				rotated+=0.15;
				if(rotated<0.0005 && rotated>-0.0005)
					rotated=0;
		}
		
	}
	else if(key=='x')
	{	
		if(rotated>-1.5)
		{
			rotateit(-0.15);
			rotated+=-0.15;
			if(rotated<0.0005 && rotated>-0.0005)
					rotated=0;
		}
		
	}
	
	else if(key==GLUT_KEY_LEFT)
	{	
		
		if(directhetha<1.5)
		{
			directhetha+=change_direction_factor;
			changedirection(change_direction_factor);
		}
		
		
		move=false;
	}
	else if(key==GLUT_KEY_RIGHT)
	{
		if(directhetha>-1.5)
		{
			directhetha-=change_direction_factor;
			changedirection(-change_direction_factor);
		}
		
	    	move=false;
	}
	else if(key==GLUT_KEY_UP)
	{  
		
		if(batwing->triangles[841].a->y<25)
		{
			
			for (int i=0; i<batwing->numVertices; ++i) 
				{
		
				batwing->vertices[i].y+=0.1;
		    		
		    		}
			    	
		 			
	    	}
		move=false;
		
	}
	else if(key==GLUT_KEY_DOWN)
	{
		
		if(batwing->triangles[841].a->y>2)
		{
			for (int i=0; i<batwing->numVertices; ++i) 
			{
		
			batwing->vertices[i].y-=0.1;
	    		
	    		}	
	    		
    		}
    		move=false;
	}
	
}
//---------------------------End of function keyOperations-----------------------------

//--------------------------------buildings---------------------------------------------------------
#define bWidth 64
#define bHeight 64
static GLubyte checkImage[bHeight][bWidth][4];
static GLuint texName,texName1;
std::list<polygon*> listofpolygons;
float base_y=0;

void maketexture() //producing a texture for the buildings----------------------------------------
{
   int i, j, c;
    
   for (i = 0; i < bHeight; i++) {
      for (j = 0; j < bWidth; j++) {
         c = ((((i&0x8)==0)&((j&0x8))==0))*255;
         checkImage[i][j][0] = (GLubyte) c;
         checkImage[i][j][1] = (GLubyte) c;
         checkImage[i][j][2] = (GLubyte) c;
         checkImage[i][j][3] = (GLubyte) 255;
      }
   }
}

//-----------------------------------------------------------------------------------------

void drawpolygons()  //function to draw the buildings
{
	 
   	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   	glGenTextures(1, &texName1);
   	glBindTexture(GL_TEXTURE_2D, texName1);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bWidth, bHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);	
	list<polygon*>::iterator it;
	glEnable(GL_TEXTURE_2D);
   	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
   	glBindTexture(GL_TEXTURE_2D, texName1);
	
	for (it= listofpolygons.begin(); it != listofpolygons.end(); ++it) 
	{
		
		glBegin(GL_POLYGON);
		glColor3f(1,1,1);
		Vector4 v1((*it)->v1[0]-(*it)->v2[0],(*it)->v1[1]-(*it)->v2[1],(*it)->v1[2]-(*it)->v2[2],0);
		Vector4 v2((*it)->v3[0]-(*it)->v4[0],(*it)->v3[1]-(*it)->v4[1],(*it)->v3[2]-(*it)->v4[2],0);
		Vector4 v=CrossProduct(v1,v2);
		glNormal3f(1*v.get_x(),1*v.get_y(),1*v.get_z());
		glTexCoord2f (0.0, 0.0);
		glVertex3fv((*it)->v1);
		glTexCoord2f (1.0, 0.0);
		glVertex3fv((*it)->v2);
		glTexCoord2f (1.0, 1.0);
		glVertex3fv((*it)->v3);
		glTexCoord2f (0.0, 1.0);
		glVertex3fv((*it)->v4);
		glEnd();
		
	}
	glDisable(GL_TEXTURE_2D);
}

void buildithere(float x,float z) //function to create a building at given location
{
	float y=base_y+5+rand() % 15;
	polygon *p;
	float f1=((rand() % 10)/10);
	float f2=((rand() % 10)/10);
	float f3=((rand() % 10)/10);
	float f4=((rand() % 10)/10);
	p=new polygon(x-f1,y,z-f3,x+5-f2,y,z+f3,x+5-f2,y,z-5+f4,x-f1,y,z-5+f4); //top
	listofpolygons.push_back(p);
	p=new polygon(x-f1,base_y,z+f3,x+5-f2,base_y,z+f3,x+5-f2,y,z+f3,x-f1,y,z+f3); //front
	listofpolygons.push_back(p);
	p=new polygon(x-f1,base_y,z+f3,x-f1,base_y,z-5+f4,x-f1,y,z-5+f4,x-f1,y,z+f3); //left
	listofpolygons.push_back(p);
	p=new polygon(x+5-f2,base_y,z+f3,x+5-f2,base_y,z-5+f4,x+5-f2,y,z-5+f4,x+5-f2,y,z+f3); //right
	listofpolygons.push_back(p);
	p=new polygon(x-f1,base_y,z-5+f4,x+5-f2,base_y,z-5+f4,x+5-f2,y,z-5+f4,x-f1,y,z-5+f4); //front //back
	listofpolygons.push_back(p);

}

void build(int Z)  //function to generate location of the building
{
	for(int z=Z;z>Z-40;z-=10)
	{
		for(int x=-20;x<20;x+=10)
		{
			buildithere(x,z);
		}
	}
	
}
//---------------------------------------------------------------------------------------
int flag=1;
int shiftgotham=30;
int zend=0;
bool bmploader();

void draw_scene() //function to generate the background scene
{
	int id;
	polygon *p;
	if(int(centerZ)==shiftgotham)
	{
		
		shiftgotham-=10;
		for(int i=0;i<20;i++)
		{
			p=listofpolygons.front();
			listofpolygons.pop_front();
			delete p;
		}
		for(int x=-20;x<20;x+=10)
		{
			buildithere(x,zend);
		}
		zend-=10;
	}
			
	glColor3f(0,0,0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texName);
        glBindTexture(GL_TEXTURE_2D, texName);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glEnable(GL_TEXTURE_2D);
  	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  	glBindTexture(GL_TEXTURE_2D, texName);
	glBegin(GL_POLYGON);
	glVertex3f (40,base_y,eyeZ+10);
	glVertex3f (-40,base_y,eyeZ+10);
	glVertex3f (-40,base_y,zend);
	glVertex3f (40,base_y,zend);
	glEnd();
	glDisable(GL_TEXTURE_2D);
    	drawpolygons();
    	glFlush ();

}
//------------------------------missiles-------------------------------------------------

struct missile
{	 
float m[3][3];
	
};
missile *center=NULL,*leftt=NULL,*rightt=NULL;

bool m_center_flag=true;
bool m_left_flag=true;
bool m_right_flag=true;

list<missile*> history_center;
list<missile*> history_left;
list<missile*> history_right;

void drawmissile(float v[3][3]) // function to draw the missiles
{
		
		
	glBegin(GL_POLYGON);
	glColor3f(1,0,0);
		
	glVertex3fv(v[0]);
	glVertex3fv(v[1]);
	glVertex3fv(v[2]);
		
	glEnd();
}

	void update_center_m() // function to update position of center missile
	{
		missile *h=new missile;	
		float x_c=(batwing->triangles[151].a->x+batwing->triangles[556].a->x)/2+direcX*0.8;
		float y_c=batwing->triangles[841].a->y;
		float z_c=(batwing->triangles[151].a->z+batwing->triangles[556].a->z)/2+direcZ*0.8;
		
		float x_l=batwing->triangles[151].a->x+direcX*1.3;
		float y_l=batwing->triangles[841].a->y;
		float z_l=batwing->triangles[151].a->z+direcZ*1.3;
		
		float x_r=batwing->triangles[556].a->x+direcX*1.3;
		float y_r=batwing->triangles[841].a->y;
		float z_r=batwing->triangles[556].a->z+direcZ*1.3;
		
		
		h->m[0][0]=x_c,h->m[0][1]=y_c,h->m[0][2]=z_c;
		h->m[1][0]=x_l,h->m[1][1]=y_l,h->m[1][2]=z_l;
		h->m[2][0]=x_r,h->m[2][1]=y_r,h->m[2][2]=z_r;
		
		if(history_center.size()==center_m_lag)
		{
			center=history_center.front();
			history_center.pop_front();
			history_center.push_back(h);
			drawmissile(center->m);
			
			
		}
		else
		{
			center=h;
			history_center.push_back(h);
			drawmissile(center->m);
		}
	}
	
	void update_left_m() // function to update position of left missile
	{	
		missile *h=new missile;	
		float x_c=(batwing->triangles[101].a->x+batwing->triangles[53].a->x)/2+direcX*0.8;
		float y_c=batwing->triangles[841].a->y;
		float z_c=(batwing->triangles[101].a->z+batwing->triangles[53].a->z)/2+direcZ*0.8;
		
		float x_l=batwing->triangles[101].a->x+direcX*0.8;
		float y_l=batwing->triangles[841].a->y;
		float z_l=batwing->triangles[101].a->z+direcZ*0.8;
		
		float x_r=batwing->triangles[53].a->x+direcX*0.3;
		float y_r=batwing->triangles[841].a->y;
		float z_r=batwing->triangles[53].a->z+direcZ*0.3;
		
		h->m[0][0]=x_c,h->m[0][1]=y_c,h->m[0][2]=z_c;
		h->m[1][0]=x_l,h->m[1][1]=y_l,h->m[1][2]=z_l;
		h->m[2][0]=x_r,h->m[2][1]=y_r,h->m[2][2]=z_r;
		
		if(history_left.size()==left_m_lag)
		{
			leftt=history_left.front();
			history_left.pop_front();
			history_left.push_back(h);
			drawmissile(leftt->m);
			
		}
		else
		{
			leftt=h;
			history_left.push_back(h);
			drawmissile(leftt->m);
		}
	}
	
	void update_right_m() // function to update position of right missile
	{	
	
		missile *h=new missile,*curr;	
		float x_c=(batwing->triangles[459].a->x+batwing->triangles[507].a->x)/2+direcX*0.8;
		float y_c=batwing->triangles[841].a->y;
		float z_c=(batwing->triangles[459].a->z+batwing->triangles[507].a->z)/2+direcZ*0.8;
		
		float x_l=batwing->triangles[459].a->x+direcX*0.3;
		float y_l=batwing->triangles[841].a->y;
		float z_l=batwing->triangles[459].a->z+direcZ*0.3;
		
		float x_r=batwing->triangles[507].a->x+direcX*0.8;
		float y_r=batwing->triangles[841].a->y;
		float z_r=batwing->triangles[507].a->z+direcZ*0.8;
		
		h->m[0][0]=x_c,h->m[0][1]=y_c,h->m[0][2]=z_c;
		h->m[1][0]=x_l,h->m[1][1]=y_l,h->m[1][2]=z_l;
		h->m[2][0]=x_r,h->m[2][1]=y_r,h->m[2][2]=z_r;
		
		if(history_right.size()==right_m_lag)
		{
			rightt=history_right.front();
			history_right.pop_front();
			history_right.push_back(h);
			drawmissile(rightt->m);
			
		}
		else
		{
			rightt=h;
			history_right.push_back(h);
			drawmissile(rightt->m);
		}
	}
	

void update_camera(float r) //function to update the camera
{	

	glLoadIdentity();
	gluPerspective(fov,aspectratio,near,far);
	
	centerX=batwing->triangles[841].a->x;
 	centerY=batwing->triangles[841].a->y;
 	centerZ=batwing->triangles[841].a->z;
  	eyeX=batwing->triangles[841].a->x;
  	eyeY=batwing->triangles[841].a->y+1/*+5*/;
 	eyeZ=batwing->triangles[841].a->z+5;
 
 	direcX=batwing->triangles[137].a->x-((batwing->triangles[556].a->x+batwing->triangles[151].a->x)/2);
  	//direcY=batwing->triangles[137].a->y-batwing->triangles[841].a->y;
  	direcY=0;
   	direcZ=batwing->triangles[137].a->z-((batwing->triangles[556].a->z+batwing->triangles[151].a->z)/2);
   	
   	Vector4 v(direcX,direcY,direcZ,0);
   	v.normalize();
   	direcX=v.get_x();
    	direcY=v.get_y();
     	direcZ=v.get_z();
	//LookAt(eyeX,eyeY,eyeZ,centerX,centerY,centerZ,upX,upY,upZ);
	float x1,y1,z1,x2,y2,z2;
	//glLoadIdentity();
	x1=batwing->triangles[841].a->x;
	y1=batwing->triangles[841].a->y;
	z1=batwing->triangles[841].a->z;
	//gluPerspective(fov,aspectratio,near,far);
	rotate(x1,y1,z1,0,1,0,eyeX,eyeY,eyeZ,r,x2,y2,z2);
  	eyeX=x2;
  	eyeY=y2;
 	eyeZ=z2;
 	upX=0/*batwing->triangles[841].a->x-eyeX*/;
 	upY=1/*batwing->triangles[841].a->y+20-eyeY*/;
 	upZ=0/*batwing->triangles[841].a->z-eyeZ*/;
	gluLookAt(eyeX,eyeY,eyeZ,centerX,centerY,centerZ,upX,upY,upZ);	
}

//-----------------------------function to detect collisions---------------------------
bool bat_building_collision()
{
	list<polygon*>::iterator it;
	for (it= listofpolygons.begin(); it != listofpolygons.end(); it++) 
	{
		
		float x_bat_left=batwing->triangles[101].a->x;
		float y_bat_left=batwing->triangles[101].a->y;
		float z_bat_front=batwing->triangles[151].a->z;
		float x_bat_right=batwing->triangles[507].a->x;
		float y_bat_right=batwing->triangles[507].a->y;
		float z_bat_back=batwing->triangles[53].a->z;
		
		float x_plane_left=(*it)->v1[0];
		float y_plane=(*it)->v1[1];
		float z_plane_front=(*it)->v1[2];
		float x_plane_right=(*it)->v2[0];
		float z_plane_back=(*it)->v3[2];
		
		
	
		if (z_bat_front>z_plane_front || z_bat_back<z_plane_back)
			{//printf("z:no col\n");
			}
			
		else
		{
			if (y_bat_left>y_plane && y_bat_right>y_plane)
				{//printf("y:no col\n");
				}
			else
			{
				if (((x_bat_left-x_plane_left)*(x_bat_left-x_plane_right)>0)&&((x_bat_right-x_plane_left)*(x_bat_right-x_plane_right)>0))
					{//printf("x:no col\n");
					}					
				else
					return true;	
			}
		}	
	it++;
	it++;
	it++;
	it++;
	}
return false;
}

bool missile_building_collision(missile *mis)
{
	list<polygon*>::iterator it;
	for (it= listofpolygons.begin(); it != listofpolygons.end(); it++) 
	{
		
		float x_missile_left=mis->m[1][0];
		float y_missile_left=mis->m[1][1];
		float z_missile_front=mis->m[0][2];
		float x_missile_right=mis->m[2][0];
		float y_missile_right=mis->m[2][1];
		float z_missile_back=mis->m[1][2];
		
		float x_plane_left=(*it)->v1[0];
		float y_plane=(*it)->v1[1];
		float z_plane_front=(*it)->v1[2];
		float x_plane_right=(*it)->v2[0];
		float z_plane_back=(*it)->v3[2];
		
		
	
		if (z_missile_front>z_plane_front || z_missile_back<z_plane_back)
			{//printf("z:no col\n");
			}
		else
		{
			if (y_missile_left>y_plane && y_missile_right>y_plane)
				{//printf("y:no col\n");
				}
			else
			{
				if (((x_missile_left-x_plane_left)*(x_missile_left-x_plane_right)>0)&&((x_missile_right-x_plane_left)*(x_missile_right-x_plane_right)>0))
					{//printf("x:no col\n");
					}					
				else
				{
					
					return true;	
				}
			}
		}	
	it++;
	it++;
	it++;
	it++;
	}
	return false; //no collision
}

//---------------------------------function to handle game end events-------------------------------
void game_failure()
{
	 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();
  	gluOrtho2D(0, 2000, 0, 2000);
  	glEnable(GL_LINE_SMOOTH);
  	glEnable(GL_BLEND);
  	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  	glLineWidth(4.0);
  
  	glColor3f(1.0, 1.0, 1.0);
  	int len, i;

  	glPushMatrix();
  
  	glTranslatef(700, 900, 0);
  	selectMessage(3);
  	len = (int) strlen(message);
  	for (i = 0; i < len; i++) {
    		glutStrokeCharacter(font, message[i]);
  	}
  	glPopMatrix();
	glFlush ();
	game_end=true;
}

void game_success()
{
	 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();
  	gluOrtho2D(0, 2000, 0, 2000);
  	glEnable(GL_LINE_SMOOTH);
  	glEnable(GL_BLEND);
  	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  	glLineWidth(4.0);
  
  	glColor3f(1.0, 1.0, 1.0);
  	int len, i;

  	glPushMatrix();
  
  	glTranslatef(700, 900, 0);
  	selectMessage(4);
  	len = (int) strlen(message);
  	for (i = 0; i < len; i++) {
    		glutStrokeCharacter(font, message[i]);
  	}
  	glPopMatrix();
	glFlush ();
	game_end=true;
}

void game_time_out()
{
	 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();
  	gluOrtho2D(0, 2000, 0, 2000);
  	glEnable(GL_LINE_SMOOTH);
  	glEnable(GL_BLEND);
  	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  	glLineWidth(4.0);
  
  	glColor3f(1.0, 1.0, 1.0);
  	int len, i;

  	glPushMatrix();
  
  	glTranslatef(700, 900, 0);
  	selectMessage(2);
  	len = (int) strlen(message);
  	for (i = 0; i < len; i++) {
    		glutStrokeCharacter(font, message[i]);
  	}
  	glPopMatrix();
  	//--------------------------------
	glFlush ();
	game_end=true;
}

void timer(int value) //the timer function
{
	if(game_end)
		return;
		//balancing the bat
	if(rotated<0)
    	{	//cout<<"r+\n";
    		rotateit(0.005);
    		rotated+=0.005;
    		if(fabs(rotated)<0.000005)
					rotated=0;
    	}
    	else if(rotated>0)
    	{
    		//cout<<"r-\n";
    		rotateit(-0.005);
    		rotated-=0.005;
    		if(fabs(rotated)<0.0000005)
					rotated=0;
    	}
    	if(move)
    	{
    		if(batwing->triangles[459].a->x<=-25)
    		{
    			directhetha-=0.01;
			changedirection(-0.01);
    		}	
    		else if(batwing->triangles[459].a->x>=25)
    		{
    			directhetha+=0.01;
			changedirection(0.01);
    		}
    		for (int i=0; i<batwing->numVertices; ++i) 
		{
		batwing->vertices[i].x-=0.1*direcX;
		batwing->vertices[i].y-=0.1*direcY;
		batwing->vertices[i].z-=0.1*direcZ;
    		
    		}
		
	}
	move=true;
	

	Time-=0.01;
	
		glutPostRedisplay();
	
		
	glutTimerFunc(10,timer,Time);
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	

	update_camera(directhetha);
		
	draw_scene();
	
	
	if(m_center_flag)
		update_center_m();
		
	
	if(m_right_flag)
		update_right_m();

	if(m_left_flag)
		update_left_m();
	
	int id;
	
	for (int i=0; i<batwing->numTriangles; ++i) 
	{
		Vector4 v=get_normal(batwing,i);
		glBegin(GL_POLYGON);
		glColor3f(1,0,0);
        	glNormal3f(-1*v.get_x(),-1*v.get_y(),-1*v.get_z());
		id=(batwing->triangles[i].a)->id;
        	glVertex3f (batwing->vertices[id].x, batwing->vertices[id].y,batwing->vertices[id].z);
        	id=(batwing->triangles[i].b)->id;
        	glVertex3f (batwing->vertices[id].x, batwing->vertices[id].y,batwing->vertices[id].z);
        	id=(batwing->triangles[i].c)->id;
        	glVertex3f (batwing->vertices[id].x, batwing->vertices[id].y,batwing->vertices[id].z);
    		glEnd();
    	}
  
    	
    	//checking bat collision with buidings
    	
    	if(bat_building_collision())
    	{
    		//cout<<"display game failed in opengl!\n";
    		 game_failure();
    	
    	}
    	//checking missile collisions with buildings
    	if(missile_building_collision(center))
    	{
    		m_center_flag=false;
    	}
    	if(missile_building_collision(leftt))
    	{
    		m_left_flag=false;
    	}
    	if(missile_building_collision(rightt))
    	{
    		m_right_flag=false;
    	}
    	
    	//checking if all missile collided
    	if(!m_center_flag && !m_left_flag && !m_right_flag)
    	{
    		//cout<<"game ended...display time taken to finish the game!\n";
    		game_success();
    	}
    	
    	if(Time<0)
    	{
    		//cout<<"time out\n";
    		game_time_out();
    	
    	}
    	
    	
	// displaying time left
    	glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();
  	gluOrtho2D(0, 2000, 0, 2000);
  	glEnable(GL_LINE_SMOOTH);
  	glEnable(GL_BLEND);
  	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  	glLineWidth(4.0);
  
  	glColor3f(1.0, 1.0, 1.0);
  	int len, i;

  	glPushMatrix();
  
  	glTranslatef(900, 1800, 0);
  	selectMessage(1);
  	len = (int) strlen(message);
  	for (i = 0; i < len; i++) {
    		glutStrokeCharacter(font, message[i]);
  	}
  	glPopMatrix();
	glFlush ();
	
    	
}
//------------------------------End of function display--------------------------------

//---------------------------------lighting effects------------------------------------
void lighting_on()
{
	
	GLfloat mat_specular[] = { 0.0, 0.5, 0.0, 0.0 };
	GLfloat mat_shininess[] = { 20.0 };
	glShadeModel (GL_SMOOTH);
	GLfloat light_ambient[] = { 0.0, 0.5, 0.5, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 0.5, 0.0, 1.0 };
	GLfloat light_specular[] = { 0.0, 0.5, 0.5, 1.0 };
	GLfloat light_position[] = { 0.0, -10.0, -200.0, 0.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
   	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

   	glEnable(GL_LIGHTING);
   	glEnable(GL_LIGHT0);
   	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
}

//-----------------------------end of lighting effects---------------------------------


void scale_down(IFS_DATA *ifs_data)
{
		for (int i=0; i<ifs_data->numVertices; ++i) 
		{
			Vector4 v(ifs_data->vertices[i].x, ifs_data->vertices[i].y,ifs_data->vertices[i].z,1);
			v=RotateY(v,180);
			ifs_data->vertices[i].x=v.get_x();
			ifs_data->vertices[i].y=v.get_y();
			ifs_data->vertices[i].z=v.get_z();
		}
		
		float m[4][4]={{0.5,0,0,0},{0,0.5,0,0},{0,0,0.5,0},{0,0,0,1}};
		Matrix4 M(m);
		
		for (int i=0; i<ifs_data->numVertices; ++i) 
		{
			Vector4 v(ifs_data->vertices[i].x, ifs_data->vertices[i].y,ifs_data->vertices[i].z,1);
			v=M*v;
			ifs_data->vertices[i].x=v.get_x();
			ifs_data->vertices[i].y=v.get_y();
			ifs_data->vertices[i].y+=15;
			ifs_data->vertices[i].z=v.get_z();
			ifs_data->vertices[i].z+=45;
		}
 

}


bool bmploader() //function to load bmp image
{
	
	
	unsigned char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins
	unsigned int imageSize;   // = width*height*3
	
	FILE * file = fopen("marble10.bmp","rb");
	if (!file)                              {printf("Image could not be opened\n"); return 0;}
	if ( fread(header, 1, 54, file)!=54 )
	{ 
		// If not 54 bytes read : problem
    		printf("Not a correct BMP file\n");
    		return false;
	}
	if ( header[0]!='B' || header[1]!='M' )
	{
    		printf("Not a correct BMP file\n");
    		return 0;
	}
	// Read ints from the byte array
	dataPos    = *(int*)&(header[0x0A]);	
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);
	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way
	// Create a buffer
	data = new unsigned char [imageSize]; 
	// Read the actual data from the file into the buffer
	fread(data,1,imageSize,file);
	//Everything is in memory now, the file can be closed
	fclose(file);
	// Create one OpenGL texture
  

}

//---------------------------------------main------------------------------------------
int main(int argc, char** argv)
{

	batwing=load_ifs_file("BATWING.ifs");
	
	bmploader();
	maketexture();
	scale_down(batwing);
	build(40);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB |GLUT_DEPTH |GLUT_SINGLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(200, 10);
	Win_id =glutCreateWindow("The BAT Escape!");
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glClearColor (0.52, 0.8, 1.0, 1.0);
	lighting_on();
	glutDisplayFunc(display);
	glutSpecialFunc(Controller);
	glutKeyboardFunc(Controller);
	glutTimerFunc(200,timer,Time);
	glutMainLoop();
 	return 0;
}
//------------------------------end of main----------------------------------------------
