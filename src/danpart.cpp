//  Copyright (C) 2005 Robert Kooima
//
//  THUMB is free software; you can redistribute it and/or modify it under
//  the terms of  the GNU General Public License as  published by the Free
//  Software  Foundation;  either version 2  of the  License,  or (at your
//  option) any later version.
//
//  This program  is distributed in the  hope that it will  be useful, but
//  WITHOUT   ANY  WARRANTY;   without  even   the  implied   warranty  of
//  MERCHANTABILITY  or FITNESS  FOR A  PARTICULAR PURPOSE.   See  the GNU
//  General Public License for more details.

#include <iostream>
#include <cstring>

#include <SDL/SDL_keyboard.h>

#include "ogl-opengl.hpp"
#include "ogl-uniform.hpp"

#include "danpart.hpp"

#include "app-frustum.hpp"
#include "app-event.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "app-host.hpp"
#include "app-glob.hpp"

#include "dev-mouse.hpp"
#include "dev-tracker.hpp"
#include "dev-gamepad.hpp"

#include <cudaGL.h>
#include <cuda_runtime.h>

//-----------------------------------------------------------------------------

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
/*
    if (cuGLRegisterBufferObject(vbo) != CUDA_SUCCESS)
        warn("CUDA register VBO failed");
*/
    return vbo;
}

void vbo_fini(GLuint vbo)
{
    glDeleteBuffersARB(1, &vbo);
/*
    if (cuGLUnregisterBufferObject(vbo) != CUDA_SUCCESS)
        warn("CUDA unregister VBO failed");
*/
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

void danpart::cuda_step()
{
/*
    // map OpenGL buffer object for writing from CUDA
    float4 *dptr;

    float r1, r2, r3;
    
    r1 = 0.0001 * (rand()%10000);
    r2 = 0.0001 * (rand()%10000);
    r3 = 0.0001 * (rand()%10000);
    //printf ("r1 r2 r3 %f %f %f \n",r1,r2,r3);
    targetX = sin(0.05 * anim);
    targetY = cos(0.03 * anim);
    targetZ = 0; 

    anim += 0.5;


    CUDA_SAFE_CALL(cudaGLMapBufferObject( (void**)&dptr, vbo));

    // execute the kernel
    dim3 block(8, 8, 1);
    dim3 grid(mesh_width / block.x, mesh_height / block.y, 1);
    kernel<<< grid, block>>>(dptr, d_particleData, mesh_width, mesh_height,
				max_age, anim, r1,r2,r3);

    // unmap buffer object
    CUDA_SAFE_CALL(cudaGLUnmapBufferObject( vbo));
*/
}

//-----------------------------------------------------------------------------

void danpart::data_init()
{
    size_t size = 4 * mesh_width * mesh_height * sizeof (float);

    srand(time(NULL));

    if ((h_particleData = (float *) malloc(size)))
    {
        if (cudaMalloc((void **) &d_particleData, size) == cudaSuccess)
        {
            for (int i = 0; i < mesh_width * mesh_height; ++i)
        	{
            	// set age to random ages < max age to force a respawn of the particle

            	h_particleData[4*i] = rand() % max_age; // age

            	// set all the velocities to get them off the screen

            	h_particleData[4*i+1] = -10000;
            	h_particleData[4*i+2] = -10000;
            	h_particleData[4*i+3] = -10000;
        	}

        	cudaMemcpy(d_particleData, h_particleData, size, cudaMemcpyHostToDevice);
        }
        else warn("CUDA malloc failed");
    }
    else warn("Particle buffer malloc failed");
}

//-----------------------------------------------------------------------------

danpart::danpart() :
    input(0),

    max_age    (2000),
    mesh_width (1024),
    mesh_height(1024)
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
//  const bool d = E->data.keybd.d;
//  const int  k = E->data.keybd.k;
//  const int  m = E->data.keybd.m;

    return false;
}

bool danpart::process_point(app::event *E)
{
    double *p = E->data.point.p;
    double *q = E->data.point.q;

    double P[3];
    double V[3];

    ::user->get_point(P, p, V, q);

//  printf("%f %f %f\n", P[0], P[1], P[2]);
//  printf("%f %f %f\n", V[0], V[1], V[2]);

    pos[0] = P[0] + V[0] * 5.0;
    pos[1] = P[1] + V[1] * 5.0;
    pos[2] = P[2] + V[2] * 5.0;

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
    ogl::range r(1.0, 100.0);

//    cuda_step();

    return r;
}

void danpart::lite(int frusc, const app::frustum *const *frusv)
{
}

void danpart::draw(int frusi, const app::frustum *frusp)
{
    // Clear the render target.

    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    // Apply the projection and view transformations.

     frusp->draw();
    ::user->draw();

    // Drop back to the fixed-function pipeline.

    glUseProgramObjectARB(0);

    // Draw the particle array.

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        {
            size_t size = 4 * mesh_width * mesh_height * sizeof (float);

            glVertexPointer(4, GL_FLOAT, 0, 0);
            glColorPointer (4, GL_FLOAT, 0, (GLvoid *) size);

            glColor3f(1.0, 0.5, 0.0);
    	    glPointSize(2.0);

            glDrawArrays(GL_POINTS, 0, mesh_width * mesh_height);
        }
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    // Draw the test triangle.

    glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);
    glBegin(GL_TRIANGLES);
    {
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
    }
    glEnd();
    glPopMatrix();    
}

//-----------------------------------------------------------------------------
