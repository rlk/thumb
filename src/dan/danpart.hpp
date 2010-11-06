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

#include "../ogl-opengl.hpp"
#include "../ogl-sprite.hpp"
#include "../ogl-mirror.hpp"
#include "../ogl-uniform.hpp"
#include "../app-prog.hpp"

#include <cuda.h>
#include "danglobs.cpp"

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
    bool process_click(app::event *);
    bool process_point(app::event *);
    bool process_timer(app::event *);

    double headPos[3];
    double wandPos[3];
    double headVec[3];
    double wandVec[3];
	int trackDevID;
	float state ;
       int trigger,triggerold;
       int but4,but4old;

       // audio files
       int chimes ;
       int pinkNoise ;
       int whiteNoise ;
       int harmonicAlgorithm ;


    // CUDA state.
// multpul function
    CUdevice   device;
    CUcontext  context;
    CUmodule   module;
    CUfunction funcHandPoint1;
    CUfunction funcHandPointSquars;
    GLuint     vbo;

    void cuda_init();
    void cuda_fini();
    void cuda_step();
    void cuda_stepPointSquars();


    // Particle system state.

    float anim;
    int   max_age;
    int   mesh_width;
    int   mesh_height;

    CUdeviceptr d_particleData;
    float      *h_particleData;
    //CUdeviceptr d_injectorData;
    float      h_injectorData[INJT_DATA_MUNB][INJT_DATA_ROWS][INJT_DATA_ROW_ELEM];
    //CUdeviceptr d_reflectorData;
    float      h_reflectorData[REFL_DATA_MUNB][REFL_DATA_ROWS][REFL_DATA_ROW_ELEM];
    CUdeviceptr d_debugData;
    float      *h_debugData;
	size_t sizeDebug;
	size_t sizei;
	size_t sizeRefl;
    void data_init();
    
    void draw_scene();
    void draw_triangles();
    
    ogl::sprite  *particle;
    ogl::mirror  *water;

    ogl::uniform *uniform_time;
    ogl::uniform *uniform_view_position;
    ogl::uniform *uniform_light_position;
    
public:

    danpart(int, int);
   ~danpart();

    bool process_event(app::event *);

    ogl::range prep(int, const app::frustum * const *);
    void       lite(int, const app::frustum * const *);
    void       draw(int, const app::frustum *);
};

//-----------------------------------------------------------------------------

#endif
