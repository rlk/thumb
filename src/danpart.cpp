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

//-----------------------------------------------------------------------------

#include <cuda.h>

void danpart::cuda_init()
{
    if (cuInit(0) == CUDA_SUCCESS)
    {
        int c = 0;

        cuDeviceGetCount(&c);

        printf("CUDA device count = %d\n", c);
    }
    else printf("CUDA failed to initialize.\n");
}

void danpart::cuda_fini()
{
}

//-----------------------------------------------------------------------------

danpart::danpart() : input(0)
{
    std::string input_mode = conf->get_s("input_mode");

    // Initialize the input handler.

    if      (input_mode == "tracker") input = new dev::tracker();
    else if (input_mode == "gamepad") input = new dev::gamepad();
    else                              input = new dev::mouse  ();

    cuda_init();
}

danpart::~danpart()
{
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

     frusp->draw();
    ::user->draw();

    glUseProgramObjectARB(0);

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
