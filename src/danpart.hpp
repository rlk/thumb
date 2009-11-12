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

#ifndef DANPART_HPP
#define DANPART_HPP

#include "ogl-opengl.hpp"
#include "app-prog.hpp"

#include <cuda.h>

//-----------------------------------------------------------------------------

namespace dev
{
    class input;
}

//-----------------------------------------------------------------------------

class danpart : public app::prog
{
    // Demo state.

    dev::input *input;

    // Event handlers

    bool process_keybd(app::event *);
    bool process_point(app::event *);
    bool process_timer(app::event *);

    double pos[3];

    // CUDA state.

    CUdevice   device;
    CUcontext  context;
    CUmodule   module;
    CUfunction function;
    GLuint     vbo;

    void cuda_init();
    void cuda_fini();
    void cuda_step();

    // Particle system state.

    float anim;
    int   max_age;
    int   mesh_width;
    int   mesh_height;

    CUdeviceptr d_particleData;
    float      *h_particleData;

    void data_init();

public:

    danpart();
   ~danpart();

    bool process_event(app::event *);

    ogl::range prep(int, const app::frustum * const *);
    void       lite(int, const app::frustum * const *);
    void       draw(int, const app::frustum *);
};

//-----------------------------------------------------------------------------

#endif
