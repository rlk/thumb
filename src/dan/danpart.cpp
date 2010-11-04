//  Copyright (C) 2005 Robert Kooima
//
//  THUMB is free software; you can redistribute it and/or modify it under
//  the terms of  the GNU General Public License as  published by the Free
//  Software  Foundation;  either version 2  of the  License,  or (at your
//  option) any later version.
//F
//  This program  is distributed in the  hope that it will  be useful, but
//  WITHOUT   ANY  WARRANTY;   without  even   the  implied   warranty  of
//  MERCHANTABILITY  or FITNESS  FOR A  PARTICULAR PURPOSE.   See  the GNU
//  General Public License for more details.

#include <iostream>
#include <cstring>

#include <SDL/SDL_keyboard.h>

#include "../ogl-opengl.hpp"
#include "../ogl-uniform.hpp"

#include "../app-frustum.hpp"
#include "../app-event.hpp"
#include "../app-conf.hpp"
#include "../app-user.hpp"
#include "../app-host.hpp"
#include "../app-glob.hpp"

#include "../dev-mouse.hpp"
#include "../dev-tracker.hpp"
#include "../dev-gamepad.hpp"

#include <SDL.h>
#include <cuda.h>
#include <cudaGL.h>

//My helper funtions

#include "danpart.hpp"
#include "danGlobs.cpp"
#include "danutils.cpp"
//-----------------------------------------------------------------------------

#define ALIGN_UP(offset, alignment) \
    (offset) = ((offset) + (alignment) - 1) & ~((alignment) - 1)

static void warn(const char *str)
{
    std::cerr << str << std::endl;
}

//-----------------------------------------------------------------------------

GLuint vbo_init(int w, int h)
{
    GLsizei size = 8 * w * h * sizeof (float);
    GLuint vbo;
	
    glGenBuffersARB(1, &vbo);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, 0, GL_DYNAMIC_DRAW);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    if (cuGLRegisterBufferObject(vbo) != CUDA_SUCCESS)
        warn("CUDA register VBO failed");

    return vbo;
}

void vbo_fini(GLuint vbo)
{
    glDeleteBuffersARB(1, &vbo);

    if (cuGLUnregisterBufferObject(vbo) != CUDA_SUCCESS)
        warn("CUDA unregister VBO failed");
}

//-----------------------------------------------------------------------------

void danpart::cuda_init()
{
    if (cuInit(0) == CUDA_SUCCESS)
    {
        if (cuDeviceGet(&device, 0) == CUDA_SUCCESS)
        {
            if (cuGLCtxCreate(&context, 0, device) == CUDA_SUCCESS)
            {
                if (cuModuleLoad(&module, "src/dan/danpart.ptx") == CUDA_SUCCESS)
                {
                    if (cuModuleGetFunction(&funcHandPoint1, module, "Point1") == CUDA_SUCCESS)
                    {
                        // YAY!
                    }
                    else warn("CUDA get funcHandPoint1 failed");
                    if (cuModuleGetFunction(&funcHandPointSquars, module, "PointSquars") == CUDA_SUCCESS)
                    {
                        // YAY!
                    }
                    else warn("CUDA get funcHandPointSquars failed");
                  }
                else warn("CUDA module load failed");
            }
            else warn("CUDA create context failed");
        }
        else warn("CUDA get device failed");
    }
    else warn("CUDA initialization failed");
}

void danpart::cuda_fini()
{
    if (cuCtxDestroy(context) == CUDA_SUCCESS)
    {
    }
    else warn("CUDA destroy context failed");
}
//---------------------------------------------------


void danpart::cuda_stepPointSquars()
{
    CUdeviceptr d_vbo;
    size_t size;

    float r1, r2, r3;// not curently used

    static double startTime = 0,nowTime = 0,frNum =1;//timeing varables
	double intigrate_time =1;
	// cuda timeing
	CUevent start, stop;
 	
	
	nowTime = getTimeInSecs();
	
	//first print out meaningless
	if ( (nowTime - startTime) > intigrate_time)
		{
		
		printf ("%f ms %f FR/sec  ",intigrate_time/frNum*1000,frNum/intigrate_time); startTime = nowTime;  frNum =0;
		}
	frNum++; 
	
    r1 = 0.0001 * (rand()%10000);
    r2 = 0.0001 * (rand()%10000);
    r3 = 0.0001 * (rand()%10000);

    //printf ("r1 r2 r3 %f %f %f \n",r1,r2,r3);
	
    anim += 0.5;
	
     // Map the buffer object.
	
    if (cuGLMapBufferObject(&d_vbo, &size, vbo) != CUDA_SUCCESS)
        warn("CUDA GL map buffer failed");
   // Set the kernel parameters.
	if ( (nowTime - startTime)== 0)
		{
		cuEventCreate(&start,CU_EVENT_DEFAULT); cuEventCreate(&stop,CU_EVENT_DEFAULT);
		cuEventRecord(start, 0);
		}
	
    int offset = 0;
    void *p;

    p = (void *) (size_t) d_vbo;
    ALIGN_UP(offset, __alignof(p));
    cuParamSetv(funcHandPointSquars, offset, &p, sizeof (p));
    offset += sizeof (p);

    p = (void *) (size_t) d_particleData;
    ALIGN_UP(offset, __alignof(p));
    cuParamSetv(funcHandPointSquars, offset, &p, sizeof (p));
    offset += sizeof (p);

    ALIGN_UP(offset, __alignof(mesh_width));
    cuParamSeti(funcHandPointSquars, offset, mesh_width);
    offset += sizeof (mesh_width);

    ALIGN_UP(offset, __alignof(mesh_height));
    cuParamSeti(funcHandPointSquars, offset, mesh_height);
    offset += sizeof (mesh_height);

    ALIGN_UP(offset, __alignof(max_age));
    cuParamSeti(funcHandPointSquars, offset, max_age);
    offset += sizeof (max_age);

    ALIGN_UP(offset, __alignof(anim));
    cuParamSetf(funcHandPointSquars, offset, anim);
    offset += sizeof (anim);

    ALIGN_UP(offset, __alignof(r1));
    cuParamSetf(funcHandPointSquars, offset, r1);
    offset += sizeof (r1);

    ALIGN_UP(offset, __alignof(r2));
    cuParamSetf(funcHandPointSquars, offset, r2);
    offset += sizeof (r2);

    ALIGN_UP(offset, __alignof(r3));
    cuParamSetf(funcHandPointSquars, offset, r3);
    offset += sizeof (r3);

    // Execute the kernel.


    cuParamSetSize     (funcHandPointSquars, offset);
    cuFuncSetBlockShape(funcHandPointSquars, 8, 8, 1);
    cuLaunchGrid       (funcHandPointSquars, mesh_width / 8, mesh_height / 8);

    // Unmap buffer object.

    if (cuGLUnmapBufferObject(vbo) != CUDA_SUCCESS)
	
        warn("CUDA GL unmap buffer failed");

	if ( (nowTime - startTime)== 0)
		{
		cuEventRecord(stop, 0);
 		cuEventSynchronize(stop);
 		float elapsedTime;
 		cuEventElapsedTime(&elapsedTime, start, stop);
		printf (" cudaProcTime %f \n \n", elapsedTime );
		cuEventDestroy (start ); cuEventDestroy (stop );
		}
	
}
//--------------------------------------------------
void danpart::cuda_step()
{
    CUdeviceptr d_vbo;
    size_t size;

    float r1, r2, r3;// not curently used

    static double startTime = 0,nowTime = 0,frNum =1;//timeing varables
	double intigrate_time =1;
	// cuda timeing
	CUevent start, stop;
 	
	
	nowTime = getTimeInSecs();
	
	//first print out meaningless
	if ( (nowTime - startTime) > intigrate_time)
		{
		
		printf ("%f ms %f FR/sec  ",intigrate_time/frNum*1000,frNum/intigrate_time); startTime = nowTime;  frNum =0;
		}
	frNum++; 
	// not curently used
    r1 = 0.0001 * (rand()%10000);
    r2 = 0.0001 * (rand()%10000);
    r3 = 0.0001 * (rand()%10000);
	//printf ("state= %f \n", state);

    //printf ("r1 r2 r3 %f %f %f \n",r1,r2,r3);
	
    anim += 0.2;
    //anim += 3.14159/4;
	//tracker data
  //printf("  devid %d \n",devid );
 // printf("pos  P %f %f %f\n", P[0], P[1], P[2]);
  //printf(" direc  V %f %f %f\n", V[0], V[1], V[2]);


//	 injector data 
	int injNum ;	
	h_injectorData[0][0][0] =1;// number of injectors ~ ~   ~ means dont care
 	//injector 1
	injNum =1;
	h_injectorData[injNum][1][0]=1;h_injectorData[injNum][1][1]=1.0;// type, injection ratio ie streem volume, ~
	h_injectorData[injNum][2][0]=0;h_injectorData[injNum][2][1]=3.0;h_injectorData[injNum][2][2]=0;//x,y,z position
	h_injectorData[injNum][3][0]=0.2 * (sin(anim/5));h_injectorData[injNum][3][1]=0.000;h_injectorData[injNum][3][2]=0.2 * (cos(anim/5));//x,y,z velocity direction
	//h_injectorData[injNum][3][0]=0.02 * (sin(anim));h_injectorData[injNum][3][1]=0;h_injectorData[injNum][3][2]=0.02 * (cos(anim));//x,y,z velocity
	//h_injectorData[injNum][3][0]=0.02 *0.0;h_injectorData[injNum][3][1]=0;h_injectorData[injNum][3][2]=0.02 * -1;//x,y,z velocity
	h_injectorData[injNum][4][0]=0.00;h_injectorData[injNum][4][1]=0.00;h_injectorData[injNum][4][2]=.0;//x,y,z size
	h_injectorData[injNum][5][0]=0.010;h_injectorData[injNum][5][1]=0.010;h_injectorData[injNum][5][2]=0.000;//t,u,v jiter v not implimented = speed 
	h_injectorData[injNum][6][0]=0.1;h_injectorData[injNum][6][1]=0.0;h_injectorData[injNum][6][2]=0.0;//speed jiter ~ ~
	h_injectorData[injNum][7][0]=5;h_injectorData[injNum][7][1]=5;h_injectorData[injNum][7][2]=5;//centrality of rnd distribution speed dt tu ~
	//
	// injector 2
	injNum =2;
	h_injectorData[injNum][1][0]=1;h_injectorData[injNum][1][1]=1.0;// type, injection ratio ie streem volume, ~
	h_injectorData[injNum][2][0]=0;h_injectorData[injNum][2][1]=20.5;h_injectorData[injNum][2][2]=0;//x,y,z position
	h_injectorData[injNum][3][0]=0.0;h_injectorData[injNum][3][1]=1.000;h_injectorData[injNum][3][2]=0;//x,y,z velocity drection
	h_injectorData[injNum][4][0]=3.000;h_injectorData[injNum][4][1]=3.0;h_injectorData[injNum][4][2]=0;//x,y,z size
	h_injectorData[injNum][5][0]=0.000;h_injectorData[injNum][5][1]=0.000;h_injectorData[injNum][5][2]=0.000;//t,u,v jiter v not implimented = speed 
	h_injectorData[injNum][6][0]=0.0;h_injectorData[injNum][6][1]=0.0;h_injectorData[injNum][6][2]=0.0;//speed jiter ~ ~
	h_injectorData[injNum][7][0]=5;h_injectorData[injNum][7][1]=5;h_injectorData[injNum][7][2]=5;//centrality of rnd distribution speed dt tu ~

 	//injector 1
	injNum =3;
	h_injectorData[injNum][1][0]=1;h_injectorData[injNum][1][1]=0.0;// type, injection ratio ie streem volume, ~
	h_injectorData[injNum][2][0]=0;h_injectorData[injNum][2][1]=3.0;h_injectorData[injNum][2][2]=0;//x,y,z position
	h_injectorData[injNum][3][0]=0.2 * (sin(anim/5));h_injectorData[injNum][3][1]=0.000;h_injectorData[injNum][3][2]=0.2 * (cos(anim/5));//x,y,z velocity direction
	//h_injectorData[injNum][3][0]=0.02 * (sin(anim));h_injectorData[injNum][3][1]=0;h_injectorData[injNum][3][2]=0.02 * (cos(anim));//x,y,z velocity
	//h_injectorData[injNum][3][0]=0.02 *0.0;h_injectorData[injNum][3][1]=0;h_injectorData[injNum][3][2]=0.02 * -1;//x,y,z velocity
	h_injectorData[injNum][4][0]=0.00;h_injectorData[injNum][4][1]=0.00;h_injectorData[injNum][4][2]=.0;//x,y,z size
	h_injectorData[injNum][5][0]=0.010;h_injectorData[injNum][5][1]=0.010;h_injectorData[injNum][5][2]=0.000;//t,u,v jiter v not implimented = speed 
	h_injectorData[injNum][6][0]=0.1;h_injectorData[injNum][6][1]=0.0;h_injectorData[injNum][6][2]=0.0;//speed jiter ~ ~
	h_injectorData[injNum][7][0]=5;h_injectorData[injNum][7][1]=5;h_injectorData[injNum][7][2]=5;//centrality of rnd distribution speed dt tu ~
	//
	for ( int n =1;n < h_injectorData[0][0][0] +1;n++)
		{
			// kludge to handel gimbel lock for velociys straight up			
			if (h_injectorData[n][3][0 ] ==0 && h_injectorData[n][3][2 ] ==0){ h_injectorData[n][3][0 ] += .0001;}
			// noramalize velocity vector
			float length = sqrt(h_injectorData[injNum][3][0] * h_injectorData[injNum][3][0] + h_injectorData[injNum][3][1] * h_injectorData[injNum][3][1] + h_injectorData[injNum][3][2] * h_injectorData[injNum][3][2]);
			h_injectorData[injNum][3][0]=h_injectorData[injNum][3][0]/length;h_injectorData[injNum][3][1]=h_injectorData[injNum][3][1]/length;h_injectorData[injNum][3][2]=h_injectorData[injNum][3][2]/length;
			
		}

	//copy data to device	
	{
	CUdeviceptr devPtr;
	size_t bytes;
	cuModuleGetGlobal(&devPtr, &bytes, module, "injdata");
	cuMemcpyHtoD(devPtr, h_injectorData, bytes);
	}
    //cuMemcpyHtoD(d_injectorData, h_injectorData, sizei);
	///	 injector data 
	//float reflectorNum =0;
	//int reflectI;
	//reflectI = (int) reflectorNum *3*8;//8 sets of 3 vallues
	
	h_reflectorData[0][0][0] =0;// number of reflectors ~ ~   ~ means dont care
	int reflNum;
	reflNum = 1;
	h_reflectorData[reflNum ][0][0]=1;// type, ~, ~
	h_reflectorData[reflNum ][1][0]=0;    h_reflectorData[reflNum ][1][1]= 0.0;h_reflectorData[reflNum ][1][2]=0;//x,y,z position
	h_reflectorData[reflNum ][2][0]=0.0;  h_reflectorData[reflNum ][2][1]=1;    h_reflectorData[reflNum ][2][2]=0;//x,y,z normal
	h_reflectorData[reflNum ][3][0]=100.00; h_reflectorData[reflNum ][3][1]=0.00; h_reflectorData[reflNum ][3][2]=0;//reflector radis ,~,~ 
	h_reflectorData[reflNum ][4][0]=0.000;h_reflectorData[reflNum ][4][1]=0.000;h_reflectorData[reflNum ][4][2]=0.000;//t,u,v jiter  not implimented = speed 
	h_reflectorData[reflNum ][5][0]= 1.0; h_reflectorData[reflNum ][5][1]=0.0;  h_reflectorData[reflNum ][5][2]=0.0;//reflection ~ ~
	h_reflectorData[reflNum ][6][0]=0;    h_reflectorData[reflNum ][6][1]=0;    h_reflectorData[reflNum ][6][2]=0;// not implemented yet centrality of rnd distribution speed dt tu ~
	
	//copy data to device	
	{	
	CUdeviceptr devPtr;
	size_t bytes;
	cuModuleGetGlobal(&devPtr, &bytes, module, "refldata");
	cuMemcpyHtoD(devPtr, h_reflectorData, bytes);
	}

	
     // Map the buffer object.
	
    if (cuGLMapBufferObject(&d_vbo, &size, vbo) != CUDA_SUCCESS)
        warn("CUDA GL map buffer failed");
   // Set the kernel parameters.

	if ( (nowTime - startTime)== 0)
		{
		cuEventCreate(&start,CU_EVENT_DEFAULT); cuEventCreate(&stop,CU_EVENT_DEFAULT);
		cuEventRecord(start, 0);
		}
	
    int offset = 0;
    void *p;

    p = (void *) (size_t) d_vbo;
    ALIGN_UP(offset, __alignof(p));
    cuParamSetv(funcHandPoint1, offset, &p, sizeof (p));
    offset += sizeof (p);

    p = (void *) (size_t) d_particleData;
    ALIGN_UP(offset, __alignof(p));
    cuParamSetv(funcHandPoint1, offset, &p, sizeof (p));
    offset += sizeof (p);
/*
    p = (void *) (size_t) d_injectorData;
    ALIGN_UP(offset, __alignof(p));
    cuParamSetv(funcHandPoint1, offset, &p, sizeof (p));
    offset += sizeof (p);

    p = (void *) (size_t) d_reflectorData;
    ALIGN_UP(offset, __alignof(p));
    cuParamSetv(funcHandPoint1, offset, &p, sizeof (p));
    offset += sizeof (p);
*/
    p = (void *) (size_t) d_debugData;
    ALIGN_UP(offset, __alignof(p));
    cuParamSetv(funcHandPoint1, offset, &p, sizeof (p));
    offset += sizeof (p);


    ALIGN_UP(offset, __alignof(mesh_width));
    cuParamSeti(funcHandPoint1, offset, mesh_width);
    offset += sizeof (mesh_width);

    ALIGN_UP(offset, __alignof(mesh_height));
    cuParamSeti(funcHandPoint1, offset, mesh_height);
    offset += sizeof (mesh_height);

    ALIGN_UP(offset, __alignof(max_age));
    cuParamSeti(funcHandPoint1, offset, max_age);
    offset += sizeof (max_age);

    ALIGN_UP(offset, __alignof(anim));
    cuParamSetf(funcHandPoint1, offset, anim);
    offset += sizeof (anim);

    ALIGN_UP(offset, __alignof(r1));
    cuParamSetf(funcHandPoint1, offset, r1);
    offset += sizeof (r1);

    ALIGN_UP(offset, __alignof(r2));
    cuParamSetf(funcHandPoint1, offset, r2);
    offset += sizeof (r2);

    ALIGN_UP(offset, __alignof(r3));
    cuParamSetf(funcHandPoint1, offset, r3);
    offset += sizeof (r3);

    // Execute the kernel.

    cuParamSetSize     (funcHandPoint1, offset);
    cuFuncSetBlockShape(funcHandPoint1, 8, 8, 1);
//    cuFuncSetBlockShape(funcHandPoint1, 16,16 , 1);
    cuLaunchGrid       (funcHandPoint1, mesh_width / 8, mesh_height / 8);
 
    // Unmap buffer object.

    if (cuGLUnmapBufferObject(vbo) != CUDA_SUCCESS)
	
        warn("CUDA GL unmap buffer failed");
	
	if ( (nowTime - startTime)== 0)
		{
		cuEventRecord(stop, 0);
 		cuEventSynchronize(stop);
 		float elapsedTime;
 		cuEventElapsedTime(&elapsedTime, start, stop);
		printf (" cudaProcTime %f \n \n", elapsedTime );
		cuEventDestroy (start ); cuEventDestroy (stop );
		}

	if (DEBUG ==1)
		{// debug data from .cu
		cuMemcpyDtoH  	( h_debugData, d_debugData, sizeDebug);
		printf (" cu debug first 18 location ingroups of 3 \n");
		for (unsigned int i =0; i < 18;i = i + 3)
		{
		printf( " %f %f %f \n",h_debugData[i],h_debugData[i+1],h_debugData[i+2]);
		
		}

	}
	
}

//---------------------------------------------------

//-----------------------------------------------------------------------------

void danpart::data_init()
{
	
// zeroout h_reflectorData
	for (int reflNum =0;reflNum < REFL_DATA_MUNB;reflNum++)
		{
			for (int rownum =0;rownum < REFL_DATA_ROWS;rownum++)
			{ 
			h_reflectorData[reflNum ][rownum][0]=0;
			h_reflectorData[reflNum ][rownum][1]=0;
			h_reflectorData[reflNum ][rownum][2]=0;
			}


		}

/*
data structure for reflectors
	data structure of reflectors data
	0) number of reflectors ,NU,NU
	1) type ,NU,NU
	2) x,y,z,position
	3) x,y,z normal vector
	4) x,y,z size
	5) x,y,z jitter
	6) reflection coef, NU, NU
	7) x,y,z centrality of velocity gitter
*/
// debug data malloc
    sizeDebug = 128* sizeof (float);

 


    if ((h_debugData = (float *) malloc(sizeDebug)))
    {
        if (cuMemAlloc(&d_debugData, sizeDebug) == CUDA_SUCCESS)
        {
            for (int i = 0; i < 128; ++i)
        	{
 				h_debugData[i]=0;
        	}
       	cuMemcpyHtoD(d_debugData, h_debugData, sizeDebug);
        }
        else warn("CUDA d_debugData malloc failed");
    }
    else warn("h_debugData malloc failed");


// partical data malloc
	int rowsize =7;
    size_t size = rowsize * mesh_width * mesh_height * sizeof (float);

    srand(time(NULL));


    if ((h_particleData = (float *) malloc(size)))
    {
        if (cuMemAlloc(&d_particleData, size) == CUDA_SUCCESS)
        {
            for (int i = 0; i < mesh_width * mesh_height; ++i)
        	{
            	// set age to random ages < max age to permit a respawn of the particle

            	h_particleData[rowsize*i] = rand() % max_age; // age

            	// set all the velocities to get them off the screen

            	h_particleData[rowsize*i+1] = -10000;
            	h_particleData[rowsize*i+2] = -10000;
            	h_particleData[rowsize*i+3] = -10000;

				// gen 3 random numbers for each partical
				h_particleData[rowsize*i+4] = 0.0002 * (rand()%10000) -1.0 ;
				h_particleData[rowsize*i+5] = 0.0002 * (rand()%10000) -1.0 ;
				h_particleData[rowsize*i+6] = 0.0002 * (rand()%10000) -1.0 ;
				//if (i > 1000000)printf ( " val %f \n ",distRnd1(  h_particleData[rowsize*i+4], 1));
 
        	}
       	cuMemcpyHtoD(d_particleData, h_particleData, size);
        }
        else warn("CUDA malloc failed");
    }
    else warn("Particle buffer malloc failed");
}

//-----------------------------------------------------------------------------

danpart::danpart() :
    input(0),
    anim(0),
    max_age    (2000),
    mesh_width (256),
    mesh_height(256)
{
    std::string input_mode = conf->get_s("input_mode");

    // Initialize the input handler.

    if      (input_mode == "tracker") input = new dev::tracker();
    else if (input_mode == "gamepad") input = new dev::gamepad();
    else                              input = new dev::mouse  ();

    cuda_init();
    vbo = vbo_init(mesh_width, mesh_height);
    data_init();
}

danpart::~danpart()
{
    vbo_fini(vbo);

    cuda_fini();

    if (input) delete input;
}

//-----------------------------------------------------------------------------

bool danpart::process_keybd(app::event *E)
{
  const bool d = E->data.keybd.d;
  const int  k = E->data.keybd.k;
  const int  m = E->data.keybd.m;
 printf(" d,k, m %d %d %d \n" ,d,k,m);


// d: 1 is godown o go up
// m : 1 is shift or controle
//k is key code

if (k == SDLK_p)printf ("p \n");

if (k == SDLK_UP && d==1){printf ("up down \n");state = 1;}
if (k == SDLK_UP && d==0){printf ("up up \n");state = 0;}
//printf ("state= %f \n", state);
    return false;
}

bool danpart::process_click(app::event *E)
{   
/*
    const int  b = E->data.click.b;
    const bool d = E->data.click.d;
*/
//d true is change to down false change to up
// return true clames event
//b is button number
//P position V vector p tracker in cave quadens q quaterian in cavequardinants
 
    return false;
}

bool danpart::process_point(app::event *E)
{
    double *p = E->data.point.p;
    double *q = E->data.point.q;
	trackDevID =E->data.point.i;
 
  
   	double P[3];
   	double V[3];
  	::user->get_point(P, p, V, q);
	//printf( " id pos %i %f %f %f \n",trackDevID,P[0],P[1],P[2]);
 /*
	if (trackDevID ==1)
		{
			headPos[0]=P[0];headPos[1]=P[1];headPos[2]=P[2];
			headVec[0]=V[0];headVec[1]=V[1];headVec[2]=V[2];
		}
	if (trackDevID ==0)
		{
			wandPos[0]=P[0];wandPos[1]=P[1];wandPos[2]=P[2];
			wandVec[0]=V[0];wandVec[1]=V[1];wandVec[2]=V[2];
		}

//P position V vector p position tracker in cave quadens q quaterian in cavequardinants
 

   //printf("  devid %d \n",devid );
	
 if (trackDevID ==0)
		{
  			printf("ID wandpos    %i %f %f %f", trackDevID ,wandPos[0], wandPos[1], wandPos[2]);
   			printf("  ID wandvec    %i %f %f %f\n", trackDevID ,wandVec[0], wandVec[1], wandVec[2]);
 		}
 if (trackDevID ==1)
		{
  			printf("ID headpos    %i %f %f %f", trackDevID ,headPos[0], headPos[1], headPos[2]);
   			printf("  ID headvec    %i %f %f %f\n", trackDevID ,headVec[0], headVec[1], headVec[2]);
 		}
*/
  //printf(" direc  V %f %f %f\n", V[0], V[1], V[2]);

 //   pos[0] = P[0] + V[0] * 5.0;
 //   pos[1] = P[1] + V[1] * 5.0;
 //   pos[2] = P[2] + V[2] * 5.0;

    return false;
}

bool danpart::process_timer(app::event *E)
{
//  double dt = E->data.timer.dt * 0.001;

    return false;
}

bool danpart::process_event(app::event *E)
{
    bool R = false;

    // Attempt to process the given event.

    switch (E->get_type())
    {
    case E_KEYBD: R = process_keybd(E); break;
    case E_CLICK: R = process_click(E); break;
    case E_POINT: R = process_point(E); break;
    case E_TIMER: R = process_timer(E); break;
    }

    if (R) return true;

    // Allow the application mode, the device, or the base to handle the event.

    if ((          prog::process_event(E)) ||
        (input && input->process_event(E)))

        // If the event was handled, disable the attract mode.
    {
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------

ogl::range danpart::prep(int frusc, const app::frustum *const *frusv)
{
    ogl::range r(0.1, 100.0);

    cuda_step();
	//cuda_stepPointSquars();

    return r;
}

void danpart::lite(int frusc, const app::frustum *const *frusv)
{
}

void danpart::draw(int frusi, const app::frustum *frusp)
{
	#include "dan_state.enum"

    // Clear the render target.
	
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    // Apply the projection and view transformations.

     frusp->draw();
    ::user->draw();

	glEnable(GL_DEPTH_TEST);

    // Drop back to the fixed-function pipeline.

    glUseProgramObjectARB(0);

    // Draw the particle array.
 	glPushMatrix();
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        {
            size_t size = 4 * mesh_width * mesh_height * sizeof (float);

            glVertexPointer(4, GL_FLOAT, 0, 0);
            glColorPointer (4, GL_FLOAT, 0, (GLvoid *) size);

          if (drawType == points)
              {   
     	        glPointSize(1.0);
				glEnable(GL_BLEND);
				glEnable(GL_POINT_SMOOTH);
				
 
                 glDrawArrays(GL_POINTS, 0, mesh_width * mesh_height);

				glDisable(GL_POINT_SMOOTH);
				glDisable(GL_BLEND);

              }
 
         if (drawType == lines)
             {
                 glDrawArrays(GL_LINES, 0, mesh_width * mesh_height );
             }

        }
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	glPopMatrix();
//added
    // Draw the test triangle.

    glPushMatrix();
    //glTranslatef(pos[0], pos[1], pos[2]);
     //printf(" pos[0] , pos[1] , pos[2] %f,%f,%f \n",pos[0], pos[1], pos[2]);
    glTranslatef(0, 0, 0);
for (int i =0; i<1;i++){
	    //glTranslatef(0.0000001, 0, 0);
    glBegin(GL_TRIANGLES);
    {
	float h;
	h =1.74;
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, h, 0.0f);
        glVertex3f(0.25f, h, 0.0f);
        glVertex3f(0.0f, h, 0.25f);
	h=1;
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, h, 0.0f);
        glVertex3f(0.25f, h, 0.0f);
        glVertex3f(0.0f, h, 0.25f);
	h=0;
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, h, 0.0f);
        glVertex3f(0.25f, h, 0.0f);
        glVertex3f(0.0f, h, 0.25f);

    }
    glEnd();
	}
	
    glPopMatrix();    

}

//-----------------------------------------------------------------------------
