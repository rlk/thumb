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

#include <ogl-opengl.hpp>
#include <ogl-sprite.hpp>
#include <ogl-mirror.hpp>
#include <ogl-uniform.hpp>
#include <app-prog.hpp>

#include <cuda.h>
#include "danglobs.cpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class pool;
    class node;
}

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

    bool process_key  (app::event *);
    bool process_click(app::event *);
    bool process_point(app::event *);
    bool process_axis (app::event *);
    bool process_tick (app::event *);

    int tracker_head_sensor;
    int tracker_hand_sensor;

    double joy_x;
    double joy_y;
    double wandMat[16];
    double headPos[3];
    double wandPos[3];
    double headVec[3];
    double wandVec[3];
    int trackDevID;
    float state;
    int trigger, triggerold;
    bool but1, but1old;
    bool but2, but2old;
    bool but3, but3old;
    bool but4, but4old;
    int sceneNum;
    int sceneOrder;
    int nextScene;

    // audio files
 
    int chimes;
    int pinkNoise;
    int whiteNoise;
    int harmonicAlgorithm;
    int texture_12;
    int short_sound_01a;
    int texture_17_swirls3;
    int dan_texture_13;
    int dan_texture_05;
    int dan_short_sound_04;
    int dan_ambiance_2;
    int dan_ambiance_1;
    int dan_5min_ostinato;
    int dan_10120603_Rez1;
    int dan_texture_18_rain_at_sea;
    int rain_at_sea;
    int dan_mel_amb_slower;
    int dan_texture_09;
    int dan_rain_at_sea_loop;
    int dan_10122606_sound_spray;
    int dan_10122608_sound_spray_low;
    int dan_10120600_rezS3_rez2;

    // CUDA state.

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

    double modulator[4];
    float anim;
    int   max_age;
    int disappear_age;
    float alphaControl;
    float showFrameNo;
    float lastShowFrameNo;
    double showStartTime;
    double showTime;
    double lastShowTime;
    float gravity;
    float colorFreq;

    int   mesh_width;
    int   mesh_height;
    int  draw_water_sky;

	

    CUdeviceptr d_particleData;
    float      *h_particleData;
    float       h_injectorData[INJT_DATA_MUNB][INJT_DATA_ROWS][INJT_DATA_ROW_ELEM];
    float       h_reflectorData[REFL_DATA_MUNB][REFL_DATA_ROWS][REFL_DATA_ROW_ELEM];
    CUdeviceptr d_debugData;
    float      *h_debugData;
    float old_refl_hits[128];// needs to have same length as d_debugData
    float refl_hits[128];

    size_t sizeDebug;
    size_t sizei;
    size_t sizeRefl;
    void data_init();
    void pdata_init_age(int max_age);
    void pdata_init_velocity(float vx,float vy,float vz);
    void pdata_init_rand();
    void copy_reflector(int sorce,int destination);
    void copy_injector(int, int);
    int loadStarcaveWalls(int);
    int load6wallcaveWalls(int);
    void draw_scene();
    void draw_triangles();
    void draw_wand_inj_image(int i);
    void draw_wand_refl_image(int i);
    int scene0Start;
    void scene_data_0();
    void scene_data_0_kill_audio();

    int scene1Start;
    void scene_data_1();
    void scene_data_1_kill_audio();
    int scene2Start;
    void scene_data_2();
    void scene_data_2_kill_audio();

    int scene3Start;	
    void scene_data_3();
    void scene_data_3_kill_audio();

    int scene4Start;	
    void scene_data_4();
    void scene_data_4_kill_audio();

    int which_scene;
    int old_which_scene;
    int sceneChange;

    ogl::sprite  *particle;
    ogl::mirror  *water;
    ogl::pool    *wand_pool;
    ogl::node    *node_inj_line;
    ogl::node    *node_inj_face;
    ogl::node    *node_ref_line;
    ogl::node    *node_ref_face;

    ogl::uniform *uniform_time;
    ogl::uniform *uniform_view_position;
    ogl::uniform *uniform_light_position;
    
public:

    danpart(const std::string&);
    ~danpart();

    bool process_event(app::event *);

    ogl::range prep(int, const app::frustum * const *);
    void       lite(int, const app::frustum * const *);
    void       draw(int, const app::frustum *);
};

//-----------------------------------------------------------------------------

#endif
