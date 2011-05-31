//  DANPART Copyright (C) 2010-2010 Dan Sandin and Robert Kooima
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

//-----------------------------------------------------------------------------

#include <cstdio>
#include <cstring>

#include <SDL.h>
#include <SDL/SDL_keyboard.h>

#include <ogl-opengl.hpp>
#include <ogl-uniform.hpp>
#include <ogl-pool.hpp>

#include <app-frustum.hpp>
#include <app-event.hpp>
#include <app-conf.hpp>
#include <app-user.hpp>
#include <app-host.hpp>
#include <app-glob.hpp>

#include <dev-mouse.hpp>
#include <dev-hybrid.hpp>
#include <dev-trackd.hpp>
#include <dev-gamepad.hpp>
#include <etc-math.hpp>

//-----------------------------------------------------------------------------

#include <cuda.h>
#include <cudaGL.h>

// Helper funtions

#include "danpart.hpp"
#include "danglobs.cpp"
#include "danutils.cpp"
#include "dansoundClient.cpp"

static int SOUND_SERV = 0;

//-----------------------------------------------------------------------------

#define ALIGN_UP(offset, alignment)                                     \
    (offset) = ((offset) + (alignment) - 1) & ~((alignment) - 1)

static void warn(const char *str)
{
    fprintf(stderr, "%s\n", str);
}

//-----------------------------------------------------------------------------

GLuint vbo_init(int w, int h)
{
    GLsizei size = 8 * w * h * sizeof (float);
    GLuint vbo;
	
    glGenBuffersARB(1,                  &vbo);
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
        if (cuDeviceGet(&device, ::host->get_device()) == CUDA_SUCCESS)
        {
            if (cuGLCtxCreate(&context, 0, device) == CUDA_SUCCESS)
            {
                if (cuModuleLoad(&module, "danpart.ptx") == CUDA_SUCCESS)
                {
                    if (cuModuleGetFunction(&funcHandPoint1, module, "Point1") == CUDA_SUCCESS)
                    {
                        // YAY!
                    }
                    else warn("CUDA get funcHandPoint1 failed");
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

//-----------------------------------------------------------------------------

void danpart::cuda_step()
{
    CUdeviceptr devPtr;
    CUdeviceptr d_vbo;
    size_t size;

    // not currently used

    float r1;
    float r2;
    float r3 = 0.0001 * (rand() % 10000);

    // Timeng varables

    static double startTime = 0, nowTime = 0, frNum = 1;
    double integrate_time = 1;

    // Timing

    showTime = getTimeInSecs() - showStartTime;
    showFrameNo++;

    // CUDA timing

    CUevent start;
    CUevent stop;
 	
    nowTime = getTimeInSecs();
	
    if ((nowTime - startTime) > integrate_time)
    {
        if (FR_RATE_PRINT > 0)
            printf("%f ms %f FR/sec",integrate_time/frNum*1000, frNum/integrate_time);
        startTime = nowTime;
        frNum = 0;
    }
    frNum++;

    // Handle scene change.

    sceneChange = 0;
    
    if ((but2old == 0) && (but2 == 1) && (but4))
    {
        sceneOrder  = (sceneOrder + 1) % 5;
        sceneChange = 1;
    }

    if (nextScene == 1)
    {
        sceneOrder  = (sceneOrder + 1) % 5;
        sceneChange = 1;
        nextScene   = 0;
    }

    // reordering scenes

    if (sceneOrder == 0) sceneNum = 4;
    if (sceneOrder == 1) sceneNum = 1;
    if (sceneOrder == 2) sceneNum = 2;
    if (sceneOrder == 3) sceneNum = 0;
    if (sceneOrder == 4) sceneNum = 3;

    if ((sceneChange == 1) && (which_scene == 0)) { scene_data_0_kill_audio(); }
    if ((sceneChange == 1) && (which_scene == 1)) { scene_data_1_kill_audio(); }
    if ((sceneChange == 1) && (which_scene == 2)) { scene_data_2_kill_audio(); }
    if ((sceneChange == 1) && (which_scene == 3)) { scene_data_3_kill_audio(); }
    if ((sceneChange == 1) && (which_scene == 4)) { scene_data_4_kill_audio(); }

    if (sceneNum == 0) // Paint on walls
    {
        if (sceneChange == 1)
        {
            scene0Start = 1;
            sceneChange = 0;
            which_scene = 0;
        }
        scene_data_0();
    }
    if (sceneNum == 1) // Spiral fountainns
    {
        if (sceneChange == 1)
        {
            scene1Start = 1;
            sceneChange = 0;
            which_scene = 1;
        }
        scene_data_1();
    }
    if (sceneNum == 2) // Four waterfalls
    {
        if (sceneChange == 1)
        {
            scene2Start = 1;
            sceneChange = 0;
            which_scene = 2;
        }
        scene_data_2();
    }
    if (sceneNum == 3) // Painting skys
    {
        if (sceneChange == 1)
        {
            scene3Start = 1;
            sceneChange = 0;
            which_scene = 3;
        }
        scene_data_3();
    }
    if (sceneNum == 4) // Rain
    {
        if (sceneChange == 1)
        {
            scene4Start = 1;
            sceneChange = 0;
            which_scene = 4;
        }
        scene_data_4();
    }

    // Kludge to handle gimble lock for velocities straight up.

    for (int n = 1;n < h_injectorData[0][0][0] +1;n++)
    {
        if (h_injectorData[n][3][0] == 0 && h_injectorData[n][3][2] == 0)
        {
            h_injectorData[n][3][0] += .0001;
        }
    }

    // Copy injector data to device.

    cuModuleGetGlobal(&devPtr, &size, module, "injdata");
    cuMemcpyHtoD(devPtr, h_injectorData, size);

    // Copy reflector data to device.

    cuModuleGetGlobal(&devPtr, &size, module, "refldata");
    cuMemcpyHtoD(devPtr, h_reflectorData, size);

    // Process audio fades.

    if ((SOUND_SERV == 1)&& (::host->root() == 1))
        audioProcess();
	
    // Map the buffer object.
	
    if (cuGLMapBufferObject(&d_vbo, &size, vbo) != CUDA_SUCCESS)
        warn("CUDA GL map buffer failed");

    // Start the timer.

    if ((nowTime - startTime) == 0)
    {
        cuEventCreate(&start,CU_EVENT_DEFAULT); cuEventCreate(&stop,CU_EVENT_DEFAULT);
        cuEventRecord(start, 0);
    }

    if (REFL_HITS == 1)
    {
        for (int i = 0; i < 128; ++i)
        {
            h_debugData[i] = 0;
        }
        cuMemcpyHtoD(d_debugData, h_debugData, sizeDebug);
    }

    // Set the kernel parameters.

    int offset = 0;

    ALIGN_UP(offset, __alignof(d_vbo));
    cuParamSetv(funcHandPoint1, offset, &d_vbo, sizeof (d_vbo));
    offset += sizeof (d_vbo);

    ALIGN_UP(offset, __alignof(d_particleData));
    cuParamSetv(funcHandPoint1, offset, &d_particleData, sizeof (d_particleData));
    offset += sizeof (d_particleData);

    ALIGN_UP(offset, __alignof(d_debugData));
    cuParamSetv(funcHandPoint1, offset, &d_debugData, sizeof (d_debugData));
    offset += sizeof (d_debugData);

    ALIGN_UP(offset, __alignof(mesh_width));
    cuParamSeti(funcHandPoint1, offset, mesh_width);
    offset += sizeof (mesh_width);

    ALIGN_UP(offset, __alignof(mesh_height));
    cuParamSeti(funcHandPoint1, offset, mesh_height);
    offset += sizeof (mesh_height);

    ALIGN_UP(offset, __alignof(max_age));
    cuParamSeti(funcHandPoint1, offset, max_age);
    offset += sizeof (max_age);

    ALIGN_UP(offset, __alignof(disappear_age));
    cuParamSeti(funcHandPoint1, offset, disappear_age);
    offset += sizeof (disappear_age);

    ALIGN_UP(offset, __alignof(alphaControl));
    cuParamSetf(funcHandPoint1, offset, alphaControl);
    offset += sizeof (alphaControl);

    ALIGN_UP(offset, __alignof(anim));
    cuParamSetf(funcHandPoint1, offset, anim);
    offset += sizeof (anim);

    ALIGN_UP(offset, __alignof(gravity));
    cuParamSetf(funcHandPoint1, offset, gravity);
    offset += sizeof (r1);

    ALIGN_UP(offset, __alignof(colorFreq));
    cuParamSetf(funcHandPoint1, offset, colorFreq);
    offset += sizeof (r2);

    ALIGN_UP(offset, __alignof(r3));
    cuParamSetf(funcHandPoint1, offset, r3);
    offset += sizeof (r3);

    // Execute the kernel.

    cuParamSetSize     (funcHandPoint1, offset);
    cuFuncSetBlockShape(funcHandPoint1, 8, 8, 1);
    cuLaunchGrid       (funcHandPoint1, mesh_width / 8, mesh_height / 8);
 
    // Unmap buffer object.

    if (cuGLUnmapBufferObject(vbo) != CUDA_SUCCESS)
        warn("CUDA GL unmap buffer failed");
	
    if ((nowTime - startTime) == 0)
    {
        float elapsedTime;

        cuEventRecord(stop, 0);
        cuEventSynchronize(stop);
        cuEventElapsedTime(&elapsedTime, start, stop);

        if (FR_RATE_PRINT >0) printf(" cudaProcTime %f \n \n", elapsedTime );

        cuEventDestroy(start);
        cuEventDestroy(stop);
    }

    if (DEBUG == 1) // debug data from .cu
    {
        cuMemcpyDtoH  	(h_debugData, d_debugData, sizeDebug);
        printf (" cu debug first 18 location ingroups of 3 \n");

        for (unsigned int i = 0; i < 18;i = i + 3)
            if (DEBUG_PRINT > 0)
                printf(" %f %f %f\n",
                       h_debugData[i],
                       h_debugData[i + 1],
                       h_debugData[i + 2]);
    }

    if (REFL_HITS == 1) // debug data from .cu
    {
        cuMemcpyDtoH(h_debugData, d_debugData, sizeDebug);
    }

    but4old = but4;
    but3old = but3;
    but2old = but2;
    but1old = but1;
    triggerold = trigger;
    lastShowTime = showTime;
    lastShowFrameNo = showFrameNo;
}

//-----------------------------------------------------------------------------

void danpart::pdata_init_age(int max_age)
{
    // Set age to random ages < max age to permit a respawn of the particle

    for (int i = 0; i < CUDA_MESH_WIDTH * CUDA_MESH_HEIGHT; ++i)
    {
        h_particleData[PDATA_ROW_SIZE*i] = rand() % max_age;
    }
}

void danpart::pdata_init_velocity(float vx,float vy,float vz)
{
    for (int i = 0; i < CUDA_MESH_WIDTH * CUDA_MESH_HEIGHT; ++i)
    { 
        h_particleData[PDATA_ROW_SIZE * i + 1] = vx;
        h_particleData[PDATA_ROW_SIZE * i + 2] = vy;
        h_particleData[PDATA_ROW_SIZE * i + 3] = vz;
    }
}

void danpart::pdata_init_rand()
{
    // Generate 3 random numbers for each particle
          
    for (int i = 0; i < CUDA_MESH_WIDTH * CUDA_MESH_HEIGHT; ++i)
    {
        h_particleData[PDATA_ROW_SIZE * i + 4] = 0.0002 * (rand()%10000) - 1.0;
        h_particleData[PDATA_ROW_SIZE * i + 5] = 0.0002 * (rand()%10000) - 1.0;
        h_particleData[PDATA_ROW_SIZE * i + 6] = 0.0002 * (rand()%10000) - 1.0;
    }
}

void danpart::data_init()
{
    // Zero out h_reflectorData

    for (int reflNum = 0; reflNum < REFL_DATA_MUNB; reflNum++)
    {
        for (int rownum = 0; rownum < REFL_DATA_ROWS; rownum++)
        { 
            h_reflectorData[reflNum][rownum][0] = 0;
            h_reflectorData[reflNum][rownum][1] = 0;
            h_reflectorData[reflNum][rownum][2] = 0;
        }
    }

    for (int injNum = 0; injNum < INJT_DATA_MUNB; injNum++)
    {
        for (int rownum = 0; rownum < INJT_DATA_ROWS; rownum++)
        { 
            h_injectorData[injNum][rownum][0] = 0;
            h_injectorData[injNum][rownum][1] = 0;
            h_injectorData[injNum][rownum][2] = 0;
        }
    }

    // data structure for reflectors
    // 0) number of reflectors ,NU,NU
    // 1) type ,NU,NU
    // 2) x,y,z,position
    // 3) x,y,z normal vector
    // 4) x,y,z size
    // 5) x,y,z jitter
    // 6) reflection coef, NU, NU
    // 7) x,y,z centrality of velocity gitter

    // debug data malloc

    sizeDebug = 128* sizeof (float);

    if ((h_debugData = (float *) malloc(sizeDebug)))
    {
        if (cuMemAlloc(&d_debugData, sizeDebug) == CUDA_SUCCESS)
        {
            for (int i = 0; i < 128; ++i)
            {
                h_debugData[i] = 0;
                old_refl_hits[i] = 0;
                refl_hits[i] = 0;
            }
            cuMemcpyHtoD(d_debugData, h_debugData, sizeDebug);
        }
        else warn("CUDA d_debugData malloc failed");
    }
    else warn("h_debugData malloc failed");

    // particle data malloc

    int rowsize = PDATA_ROW_SIZE;
    size_t size = rowsize * mesh_width * mesh_height * sizeof (float);

    srand(1);

    if ((h_particleData = (float *) malloc(size)))
    {
        if (cuMemAlloc(&d_particleData, size) == CUDA_SUCCESS)
        {

            pdata_init_age(max_age);
            pdata_init_velocity(-10000, -10000, -10000);
            pdata_init_rand();
            cuMemcpyHtoD(d_particleData, h_particleData, size);
        }
        else warn("CUDA malloc failed");
    }
    else warn("Particle buffer malloc failed");

    // init buttons

    trackDevID = 0;
    state = 0;
    trigger = 0;
    but4 = 0;
    but3 = 0;
    but2 = 0;
    but1 = 0;

    // init seenes

    scene0Start = 0;
    scene1Start = 0;
    scene2Start = 0;
    scene3Start = 0;
    scene4Start = 1; // must be set to starting
    sceneNum    = 0;
    sceneOrder  = 0;
    sceneChange = 0;
    nextScene   = 0;
    which_scene = 4; // must be set to starting
    old_which_scene = -1;

    // init timers

    showStartTime = getTimeInSecs();
    if (DEBUG_PRINT > 0) printf("showstartTime %f \n", showStartTime);

    // Init sound server

    if (ENABLE_SOUND_SERV && (::host->root()))
    {
        printf ("audioConectToServer ");

        if (HOST == STARCAVE) SOUND_SERV = audioConectToServer("137.110.118.239");
        if (HOST == NEXCAVE)  SOUND_SERV = audioConectToServer("10.1.1.242");

        if (SOUND_SERV)
        {
            audioGlobalGain(.7);
 
            pinkNoise                    = audioGetHandKludge("cdtds.31.pinkNoise.wav");
            audioGain(pinkNoise, 0);
            audioLoop(pinkNoise, 1);
            audioPlay(pinkNoise, 1.0);

            dan_texture_09               = audioGetHandKludge("dan_texture_09.wav");
            audioGain(dan_texture_09, 0);
            audioLoop(dan_texture_09, 1);
            audioPlay(dan_texture_09, 1.0);
 
            texture_12                   = audioGetHandKludge("dan_texture_12.wav");
            audioGain(texture_12, 0);
            audioLoop(texture_12, 1);
            audioPlay(texture_12, 1.0);
 
            dan_10122606_sound_spray     = audioGetHandKludge("dan_10122606_sound_spray.wav");
            audioGain(dan_10122606_sound_spray, 0);
            audioLoop(dan_10122606_sound_spray, 1);
            audioPlay(dan_10122606_sound_spray, 1.0);
            
            short_sound_01a              = audioGetHandKludge("dan_short_sound_01a.wav");
            audioPlay(short_sound_01a, 1);

            chimes                       = audioGetHandKludge("chimes.wav");
            texture_17_swirls3           = audioGetHandKludge("dan_texture_17_swirls3.wav");
            rain_at_sea                  = audioGetHandKludge("dan_texture_18_rain_at_sea.wav");
            dan_texture_13               = audioGetHandKludge("dan_texture_13.wav");
            dan_texture_05               = audioGetHandKludge("dan_texture_05.wav");
            dan_short_sound_04           = audioGetHandKludge("dan_short_sound_04.wav");
            dan_ambiance_2               = audioGetHandKludge("dan_ambiance_2.wav");
            dan_ambiance_1               = audioGetHandKludge("dan_ambiance_1.wav");
            dan_5min_ostinato            = audioGetHandKludge("dan_10120607_5_min_ostinato.WAV");
            dan_10120603_Rez1            = audioGetHandKludge("dan_10120603_Rez.1.wav");
            dan_mel_amb_slower           = audioGetHandKludge("dan_10122604_mel_amb_slower.wav");
            harmonicAlgorithm            = audioGetHandKludge("harmonicAlgorithm.wav");
            dan_rain_at_sea_loop         = audioGetHandKludge("dan_rain_at_sea_loop.wav");
            dan_10122608_sound_spray_low = audioGetHandKludge("dan_10122608_sound_spray_low.wav");
            dan_10120600_rezS3_rez2      = audioGetHandKludge("dan_10120600_RezS.3_Rez.2.wav");
        }
    }
}

//-----------------------------------------------------------------------------

danpart::danpart(const std::string& tag) :
    app::prog(tag),
    input(0),
    anim(0),
    max_age(2000),
    disappear_age(2000),
    showFrameNo(0),
    lastShowFrameNo(-1),
    showStartTime(0),
    showTime(0),
    lastShowTime(-1),
    gravity(.0001),
    colorFreq(16),
    mesh_width (CUDA_MESH_WIDTH),
    mesh_height(CUDA_MESH_HEIGHT),
    draw_water_sky(1),
    
    particle(new ogl::sprite()),
    water(new ogl::mirror("mirror-water", ::host->get_buffer_w(),
                          ::host->get_buffer_h())),
    
    uniform_time          (::glob->load_uniform("time",             1)),
    uniform_view_position (::glob->load_uniform("view_position",    3)),
    uniform_light_position(::glob->load_uniform("light_position",   3))

{
    std::string input_mode = conf->get_s("input_mode");

    // Initialize the input handler.

    tracker_head_sensor = ::conf->get_i("tracker_head_sensor");
    tracker_hand_sensor = ::conf->get_i("tracker_hand_sensor");

    if      (input_mode == "trackd")  input = new dev::trackd();
    else if (input_mode == "hybrid")  input = new dev::hybrid("hybrid.xml");
    else if (input_mode == "gamepad") input = new dev::gamepad();
    else                              input = new dev::mouse  ();

    cuda_init();
    vbo = vbo_init(mesh_width, mesh_height);
    data_init();

    modulator[0] = 1.0;
    modulator[1] = 1.0;
    modulator[2] = 1.0;
    modulator[3] = 1.0;

    // Initialize the wand renderer pool and wand model nodes.

    wand_pool = ::glob->new_pool();

    node_inj_line = new ogl::node;
    node_inj_face = new ogl::node;
    node_ref_line = new ogl::node;
    node_ref_face = new ogl::node;

    node_inj_line->add_unit(new ogl::unit("model/particle-inj-line.obj", false));
    node_inj_face->add_unit(new ogl::unit("model/particle-inj-face.obj", false));
    node_ref_line->add_unit(new ogl::unit("model/particle-ref-line.obj", false));
    node_ref_face->add_unit(new ogl::unit("model/particle-ref-face.obj", false));

    wand_pool->add_node(node_inj_line);
    wand_pool->add_node(node_inj_face);
    wand_pool->add_node(node_ref_line);
    wand_pool->add_node(node_ref_face);
}

danpart::~danpart()
{
    vbo_fini(vbo);

    cuda_fini();

    ::glob->free_pool(wand_pool);

    ::glob->free_uniform(uniform_light_position);
    ::glob->free_uniform(uniform_view_position);
    ::glob->free_uniform(uniform_time);
    
    delete water;
    delete particle;
    
    if (input) delete input;

    if ((SOUND_SERV == 1) && (::host->root() == 1)) audioQuit();
}

//-----------------------------------------------------------------------------

bool danpart::process_key(app::event *E)
{
    const bool d = E->data.key.d;
    const int  k = E->data.key.k;
    const int  m = E->data.key.m;
    if (DEBUG_PRINT >0)printf(" d,k, m %d %d %d \n" ,d,k,m);


// d: 1 is godown o go up
// m : 1 is shift or controle
// k is key code

    if (k == SDLK_p)if (DEBUG_PRINT >0) printf("p \n");

    if (k == SDLK_UP && d == 1){if (DEBUG_PRINT >0) {printf("up down \n");}state = 1;}
    if (k == SDLK_UP && d == 0){if (DEBUG_PRINT >0) {printf("up up \n");}state = 0;}
// printf ("state = %f \n", state);
    return false;
}

bool danpart::process_click(app::event *E)
{  
    const int  b = E->data.click.b;
    const bool d = E->data.click.d;

    if (DEBUG_PRINT > 0) printf("b=%d d=%d \n",b, d);

    if ((b == 1) & (d == 1)) { but4old = but4; but4 = 1; }
    if ((b == 1) & (d == 0)) { but4old = but4; but4 = 0; }

    if ((b == 2) & (d == 1)) { but3old = but3; but3 = 1; }
    if ((b == 2) & (d == 0)) { but3old = but3; but3 = 0; }

    if ((b == 3) & (d == 1)) { but2old = but2; but2 = 1; }
    if ((b == 3) & (d == 0)) { but2old = but2; but2 = 0; }

    if ((b == 4) & (d == 1)) { but1old = but1; but1 = 1; }
    if ((b == 4) & (d == 0)) { but1old = but1; but1 = 0; }

    if ((HOST == STARCAVE) || (HOST == MAC_SIM) || (MAC_SIM_NEXCAVE))
    {
        if ((b == 0) & (d == 1)) { triggerold = trigger; trigger = 1; }
        if ((b == 0) & (d == 0)) { triggerold = trigger; trigger = 0; }
    }
    if ((HOST == NEXCAVE))
    {
        if ((b == 7) & (d == 1)) { triggerold = trigger; trigger = 1; }
        if ((b == 7) & (d == 0)) { triggerold = trigger; trigger = 0; }

    }

    return false;
}

bool danpart::process_point(app::event *E)
{
    double *p = E->data.point.p;
    double *q = E->data.point.q;
    trackDevID = E->data.point.i;
 
    const double *A = ::user->get_M();
    double B[16];  
 
    double P[3];
    double V[3];
    ::user->get_point(P, p, V, q);

    if (trackDevID == tracker_head_sensor)
    {
        headPos[0] = P[0];headPos[1] = P[1];headPos[2] = P[2];
        headVec[0] = V[0];headVec[1] = V[1];headVec[2] = V[2];
    }
    if (trackDevID == tracker_hand_sensor)
    {
    
        double t[3], o[3] = { 0.0, 0.150, 0.0 };

        quat_to_mat(B, q);
        mult_mat_vec3(t, B, o);
        mult_mat_vec3(o, A, t);

        wandPos[0] = P[0]+t[0];wandPos[1] = P[1]+t[1];wandPos[2] = P[2]+t[2];
        wandVec[0] = V[0];     wandVec[1] = V[1];     wandVec[2] = V[2];

        mult_mat_mat(wandMat, A, B);
        wandMat[12] = wandPos[0];
        wandMat[13] = wandPos[1];
        wandMat[14] = wandPos[2];
    }

    return false;
}

bool danpart::process_tick(app::event *E)
{
    double dt = E->data.tick.dt;

    if (fabs(joy_x) > 0.1)
        ::user->pass(joy_x * dt * 10.0);

    return false;
}

bool danpart::process_axis(app::event *E)
{
    if ((HOST == STARCAVE) || (HOST == MAC_SIM)||(MAC_SIM_NEXCAVE))
    {
        if (E->data.axis.a == 0) joy_x = E->data.axis.v;
        if (E->data.axis.a == 1) joy_y = E->data.axis.v;
    }
    if (HOST == NEXCAVE)
    {
        if (E->data.axis.i == 0)
        {
            if (E->data.axis.a == 0) joy_x = E->data.axis.v;
            if (E->data.axis.a == 1) joy_y = E->data.axis.v;
        }
    }

    return false;
}

bool danpart::process_event(app::event *E)
{
    bool R = false;

    // Attempt to process the given event.

    switch (E->get_type())
    {
    case E_KEY:   R = process_key  (E); break;
    case E_CLICK: R = process_click(E); break;
    case E_POINT: R = process_point(E); break;
    case E_TICK:  R = process_tick (E); break;
    case E_AXIS:  R = process_axis (E); break;
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

void danpart::draw_wand_inj_image(int i)
{
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (trigger)
            glColor4f(0.2f, 0.2f, 1.0f, 0.40f);
        else
            glColor4f(0.1f, 0.1f, 0.7f, 0.00f);

        wand_pool->draw_init();
        node_inj_line->transform(wandMat);
        node_inj_face->transform(wandMat);
        node_inj_line->draw();
        node_inj_face->draw();
        wand_pool->draw_fini();
    }
    glPopAttrib();
}

void danpart::draw_wand_refl_image(int i)
{
    if (trigger)
    {
        double M[16], R[16];
        double r = h_reflectorData[i][3][0];
        double n =     h_debugData[i];

        // Compute the wand transform.

        load_scl_mat(R, r, r, r);
        mult_mat_mat(M, wandMat, R);

        // Compute the want color.

        double k = (log10(n) / 3.0 - 0.5) * 1.5;

        if (k < 0.0) k = 0.0;
        if (k > 1.0) k = 1.0;

        glColor4f(0.2f * (1.0 - k) + 1.0f * k,
                  0.2f * (1.0 - k) + 1.0f * k,
                  1.0f * (1.0 - k) + 0.0f * k, 0.5f);

        // Draw the wand.

        glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            wand_pool->draw_init();
            {
                node_ref_line->transform(M);
                node_ref_face->transform(M);
                node_ref_line->draw();
                node_ref_face->draw();
            }
            wand_pool->draw_fini();
        }
        glPopAttrib();
    }
}


void danpart::draw_triangles()
{
    // Draw the test triangles.

    if (SHOW_MARKERS)
    {
        glPushMatrix();
        {
            glBegin(GL_TRIANGLES);
            {
                GLfloat h;
	              
                h = ftToM(5.0);
            	
                glColor3f(1.0f, 1.0f, 0.0f);
                glVertex3f(0.00f, h, 0.00f);
                glVertex3f(0.25f, h, 0.00f);
                glVertex3f(0.00f, h, 0.25f);

                h = ftToM(2.5);

                glColor3f(0.0f, 1.0f, 0.0f);
                glVertex3f(0.00f, h, 0.00f);
                glVertex3f(0.25f, h, 0.00f);
                glVertex3f(0.00f, h, 0.25f);

                h = ftToM(0.0);
            	
                glColor3f(1.0f, 1.0f, 1.0f);
                glVertex3f(0.00f, h, 0.00f);
                glVertex3f(0.25f, h, 0.00f);
                glVertex3f(0.00f, h, 0.25f);
            }
            glEnd();
        }
        glPopMatrix();
    }
}

void danpart::draw_scene()
{
#include "dan_state.enum"

    glEnable(GL_DEPTH_TEST);

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
                particle->bind();
                glDrawArrays(GL_POINTS, 0, mesh_width * mesh_height);
                particle->free();
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
}

//-----------------------------------------------------------------------------

ogl::range danpart::prep(int frusc, const app::frustum *const *frusv)
{
    ogl::range r(0.1, 100.0);

    // Set the scene uniforms.

    uniform_time->set(double(SDL_GetTicks() * 0.001));
    uniform_view_position ->set(::user->get_M() + 12);
    uniform_light_position->set(::user->get_L());

    // Run the particle simulation.

    cuda_step();

    // Prep the renderer.

    wand_pool->prep();

    return r;
}

void danpart::lite(int frusc, const app::frustum *const *frusv)
{
}

void danpart::draw(int frusi, const app::frustum *frusp)
{
    // Clear the render target.
	
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    // Apply the projection and view transformations.
 
    frusp->draw();
    ::user->draw();
  
    // Draw the water and sky.

    if (draw_water_sky == 1)
    {
        water->bind();
        {
            draw_scene();
        }
        water->free();
        water->draw(frusp);
    }
   
    // Draw the scene.

    draw_scene();
    draw_triangles();

    if (which_scene == 0) draw_wand_inj_image(1);
    if (which_scene == 1) draw_wand_refl_image(2);
    if (which_scene == 2) draw_wand_refl_image(1);
    if (which_scene == 3) draw_wand_inj_image(3);
    if (which_scene == 4) draw_wand_refl_image(1);
}

void danpart::copy_injector(int source, int destination)
{
    for (int row = 0;row < INJT_DATA_ROWS; row++)
    {
        for (int ele = 0; ele < 3; ele++)
        {
            h_injectorData[destination][row][ele] = h_injectorData[source][row][ele];
        }
    }
}

void danpart::copy_reflector(int source, int destination)
{
    for (int row = 0;row < REFL_DATA_ROWS;row++)	
    {
        for (int ele = 0;ele <3;ele++)
        {
            h_reflectorData[ destination][row][ele] = h_reflectorData[source][row][ele];
        }
    }
}

int danpart::load6wallcaveWalls(int firstRefNum)
{
    float caverad = ftToM(5.0);
    float damping = .5;
    float no_traping = 0;
    int reflNum;

    reflNum = firstRefNum;
    h_reflectorData[reflNum][0][0] =  1;
    h_reflectorData[reflNum][0][1] =  0; // type, age ie colormod, ~  0 is off 1 is plane reflector
    h_reflectorData[reflNum][3][0] =  caverad;
    h_reflectorData[reflNum][3][1] =  0.00;
    h_reflectorData[reflNum][3][2] =  0; // reflector radis ,~,~ 
    h_reflectorData[reflNum][4][0] =  0.000;
    h_reflectorData[reflNum][4][1] =  0.000;
    h_reflectorData[reflNum][4][2] =  0.000; // t,u,v jitter  not implimented = speed 
    h_reflectorData[reflNum][5][0] =  damping;
    h_reflectorData[reflNum][5][1] =  no_traping;
    h_reflectorData[reflNum][5][2] =  0.0; // reflectiondamping , no_traping ~
    h_reflectorData[reflNum][6][0] =  0;
    h_reflectorData[reflNum][6][1] =  0;
    h_reflectorData[reflNum][6][2] =  0; // not implemented yet centrality of rnd distribution speed dt tu ~

    // front
    h_reflectorData[reflNum][1][0] =  0;
    h_reflectorData[reflNum][1][1] =  caverad;
    h_reflectorData[reflNum][1][2] = -caverad; // x,y,z position
    h_reflectorData[reflNum][2][0] =  0;
    h_reflectorData[reflNum][2][1] =  0;
    h_reflectorData[reflNum][2][2] =  1; // x,y,z normal

    copy_reflector(reflNum, reflNum + 1);
    reflNum++; // back
    h_reflectorData[reflNum][1][0] =  0;
    h_reflectorData[reflNum][1][1] =  caverad;
    h_reflectorData[reflNum][1][2] =  caverad; // x,y,z position
    h_reflectorData[reflNum][2][0] =  0;
    h_reflectorData[reflNum][2][1] =  0;
    h_reflectorData[reflNum][2][2] = -1; // x,y,z normal

    copy_reflector(reflNum, reflNum + 1);
    reflNum++; // right
    h_reflectorData[reflNum][1][0] =  caverad;
    h_reflectorData[reflNum][1][1] =  caverad;
    h_reflectorData[reflNum][1][2] =  0; // x,y,z position
    h_reflectorData[reflNum][2][0] = -1;
    h_reflectorData[reflNum][2][1] =  0;
    h_reflectorData[reflNum][2][2] =  0; // x,y,z normal

    copy_reflector(reflNum, reflNum + 1);
    reflNum++; // left
    h_reflectorData[reflNum][1][0] = -caverad;
    h_reflectorData[reflNum][1][1] =  caverad;
    h_reflectorData[reflNum][1][2] =  0; // x,y,z position
    h_reflectorData[reflNum][2][0] =  1;
    h_reflectorData[reflNum][2][1] =  0;
    h_reflectorData[reflNum][2][2] =  0; // x,y,z normal

    copy_reflector(reflNum, reflNum + 1);
    reflNum++; // top
    h_reflectorData[reflNum][1][0] =  0;
    h_reflectorData[reflNum][1][1] =  2 * caverad;
    h_reflectorData[reflNum][1][2] =  0; // x,y,z position
    h_reflectorData[reflNum][2][0] =  0;
    h_reflectorData[reflNum][2][1] = -1;
    h_reflectorData[reflNum][2][2] =  0; // x,y,z normal

    copy_reflector(reflNum, reflNum + 1);
    reflNum++; // bottom
    h_reflectorData[reflNum][1][0] =  0;
    h_reflectorData[reflNum][1][1] = -0;
    h_reflectorData[reflNum][1][2] =  0; // x,y,z position
    h_reflectorData[reflNum][2][0] =  0;
    h_reflectorData[reflNum][2][1] =  1;
    h_reflectorData[reflNum][2][2] =  0; // x,y,z normal
    
    return reflNum;
}

// ////////////////// ////////////////////////////////////////////////////////////////////////////


void danpart::scene_data_0_kill_audio()
{
    h_reflectorData[0][0][0] = 0; // turn off all reflectors
    h_injectorData[0][0][0] = 0; // turn off all injectors

    if ((SOUND_SERV == 1) && (::host->root() == 1))
    {
        audioFadeOutStop(dan_ambiance_2, 150,-1);
        audioGain(dan_10122606_sound_spray,0);
    }
}

void danpart::scene_data_0()
{
    // paint on walls
// particlesysparamiteres--------------
    gravity = .1;
    gravity = 0.000001;
    // gravity = 0.0001;
    colorFreq = 16;
    max_age = 2000;
    disappear_age = 2000;
    alphaControl = 0;
    static float time_in_scene;
    time_in_scene = time_in_scene + 1.0/TARGET_FR_RATE;
	
    if (scene0Start == 1)
    {
        size_t size = PDATA_ROW_SIZE * CUDA_MESH_WIDTH * CUDA_MESH_HEIGHT * sizeof (float);
        // pdata_init_age(max_age);
        pdata_init_velocity(0, -0.005, 0);
        pdata_init_rand();
        cuMemcpyHtoD(d_particleData, h_particleData, size);
        time_in_scene = 0 * TARGET_FR_RATE;
        ::user->home();

        if (DEBUG_PRINT >0)printf("scene0Start \n");
        if ((SOUND_SERV == 1)&& (::host->root() == 1))
        {
            audioPlay(dan_ambiance_2,1.0);audioGain(dan_ambiance_2,1);
                    
        }
          
        // printf("scene0Start \n");
    }
    if (time_in_scene >90)nextScene = 1;          

// printf ("time_in_scene 0 %f\n",time_in_scene);

// printf("in scene1 \n");

    // anim += 0.001; // .0001
    anim = showFrameNo * .001;
    // anim += 3.14159/4;
    // tracker data
    // printf("  devid %d \n",devid );
    // printf("pos  P %f %f %f\n", P[0], P[1], P[2]);
    // printf(" direc  V %f %f %f\n", V[0], V[1], V[2]);

    draw_water_sky = 0;
	
// 	 injector data 

// 	 injector data 
    int injNum ;	
    h_injectorData[0][0][0] = 1; // number of injectors ~ ~   ~ means dont care
    // injector 1
    injNum = 1;

    if ((SOUND_SERV == 1)&& (::host->root() == 1))
    {
        // audioGain(texture_12,trigger);
	
        // if ((triggerold == 0) && (trigger == 1)){audioPlay(short_sound_01a,0.1);audioGain(texture_12,1);}
        if ((triggerold == 0) && (trigger == 1)){audioFadeUp(texture_12, 1, 1, short_sound_01a);}
        if ((triggerold == 1) && (trigger == 0)){audioFadeOut(texture_12, 10, -1);}

    }
    injNum = 1;
    h_injectorData[injNum][1][0] = 1;
    h_injectorData[injNum][1][1] = trigger; // type, injection ratio ie streem volume, ~
    h_injectorData[injNum][2][0] = wandPos[0];
    h_injectorData[injNum][2][1] = wandPos[1];
    h_injectorData[injNum][2][2] = wandPos[2]; // x,y,z position
    h_injectorData[injNum][3][0] = wandVec[0];
    h_injectorData[injNum][3][1] = wandVec[1];
    h_injectorData[injNum][3][2] = wandVec[2]; // x,y,z velocity direction
    h_injectorData[injNum][4][0] = 0.00;
    h_injectorData[injNum][4][1] = 0.00;
    h_injectorData[injNum][4][2] = .0; // x,y,z size
    h_injectorData[injNum][5][0] = 0.010;
    h_injectorData[injNum][5][1] = 0.010;
    h_injectorData[injNum][5][2] = 0.000; // t,u,v jitter v not implimented = speed 
    h_injectorData[injNum][6][0] = .1;
    h_injectorData[injNum][6][1] = 0.1;
    h_injectorData[injNum][6][2] = 0.0; // speed jitter ~ ~
    h_injectorData[injNum][7][0] = 5;
    h_injectorData[injNum][7][1] = 5;
    h_injectorData[injNum][7][2] = 5; // centrality of rnd distribution speed dt tu ~

    // load starcave wall reflectors
    // h_reflectorData[0][0][0] = loadStarcaveWalls(1);
    if (time_in_scene >5)h_reflectorData[0][0][0] = load6wallcaveWalls(1);

    scene0Start = 0;
}

void danpart::scene_data_1_kill_audio()
{
    h_reflectorData[0][0][0] = 0; // turn off all reflectors
    h_injectorData[0][0][0] = 0; // turn off all injectors

    if ((SOUND_SERV == 1)&& (::host->root() == 1))
    {
        audioFadeOutStop(dan_5min_ostinato, 150,-1);
        audioGain(dan_10122606_sound_spray,0);
    }
}

void danpart::scene_data_1()
{
// spiral Fountens

    draw_water_sky = 0;
// particlesysparamiteres--------------
    gravity = .00005;	
    max_age = 2000;
    disappear_age = 2000;
    colorFreq = 64 *max_age/2000.0 ;
    alphaControl = 0; // turns alph to transparent beloy y = 0
// reload  rnd < max_age in to pdata
    static float time_in_scene;
    time_in_scene = time_in_scene + 1.0/TARGET_FR_RATE;


    if (scene1Start == 1)
    {
        // size_t size = PDATA_ROW_SIZE * CUDA_MESH_WIDTH * CUDA_MESH_HEIGHT * sizeof (float);
        // pdata_init_age(max_age);
        // pdata_init_velocity(-10000, -10000, -10000);
        // pdata_init_rand();
        // cuMemcpyHtoD(d_particleData, h_particleData, size);
        time_in_scene = 0 * TARGET_FR_RATE;
        ::user->home();
        // printf("in start scene3 \n");
        if (DEBUG_PRINT >0)printf("scene0Start \n");
        if ((SOUND_SERV == 1)&& (::host->root() == 1))
        {
            audioPlay(dan_5min_ostinato,1.0);audioGain(dan_5min_ostinato,0.5);
                    
        }
 
            
        // printf(" time %f \n", ::user->get_t());
    }
// printf ("time_in_scene 1 %f\n",time_in_scene);
    if (time_in_scene >110)nextScene = 1;
    if (DEBUG_PRINT >0)printf("in scene1 \n");
// printf("in scene1 \n"); 

    // anim += 0.001; // .0001
    static float rotRate = .05;
    anim = showFrameNo * rotRate;
    rotRate += .000001;

// 	 injector data 
    int injNum ;	
    // number of injectors ~ ~   ~ means dont care
    // injector 1
    injNum = 1;
	
    h_injectorData[injNum][1][0] = 2;
    h_injectorData[injNum][1][1] = 1.0; // type, injection ratio ie streem volume, ~
    h_injectorData[injNum][2][0] = 0;
    h_injectorData[injNum][2][1] = ftToM(0.1);
    h_injectorData[injNum][2][2] = 0; // x,y,z position
    h_injectorData[injNum][3][0] = 0.02 * (sin(time_in_scene*2*PI));
    h_injectorData[injNum][3][1] = 0.5;
    h_injectorData[injNum][3][2] = 0.02 * (cos(time_in_scene*2*PI)); // x,y,z velocity
    h_injectorData[injNum][4][0] = 0.03;
    h_injectorData[injNum][4][1] = 0.03;
    h_injectorData[injNum][4][2] = 0.03; // x,y,z size
    h_injectorData[injNum][5][0] = 0.0000;
    h_injectorData[injNum][5][1] = 0.00000;
    h_injectorData[injNum][5][2] = 0.000; // t,u,v jitter v not implimented = speed 
    h_injectorData[injNum][6][0] = .03;
    h_injectorData[injNum][6][1] = 0.0;
    h_injectorData[injNum][6][2] = 0.0; // speed jitter ~ ~
    h_injectorData[injNum][7][0] = 5;
    h_injectorData[injNum][7][1] = 5;
    h_injectorData[injNum][7][2] = 5; // centrality of rnd distribution speed dt tu ~
    int reflect_on;
    reflect_on = 0;
    float speed = 1.0/30; // one rotation every 
    speed = 1.0/45;	
    float stime = 0 ;
    float t,rs,as,mr = 9.0;
	
   
    if ((HOST == STARCAVE) || (HOST == MAC_SIM))
    {
    
        int numobj = 5;
        h_injectorData[0][0][0] = numobj;


    
    	for(int i = 1;i<= numobj;i++)
    	{
            if (time_in_scene > 1.0/speed)reflect_on = 1;	
            if (time_in_scene > 1.5/speed)
            {
                reflect_on = 1;
			
                h_injectorData[injNum][3][0] = 0;
                h_injectorData[injNum][3][1] = .5;
                h_injectorData[injNum][3][2] = 0.0; // x,y,z velocity
                if (time_in_scene > 2.0/speed)
                {
                    h_injectorData[injNum][3][0] = -ftToM(rs*sin(as))/8 ;
                    h_injectorData[injNum][3][1] = .5;
                    h_injectorData[injNum][3][2] = ftToM(rs*cos(as))/8; // x,y,z velocity
            	    h_injectorData[injNum][6][0] = .026;
                    h_injectorData[injNum][6][1] = 0.0;
                    h_injectorData[injNum][6][2] = 0.0; // speed jitter ~ ~
                           
            	}
            }
    	 	
    	    copy_injector(1, i);
            stime = i*(1/speed/numobj);
            if (time_in_scene >stime)
            {
                injNum = i; t = time_in_scene - stime;rs = 2*speed*(2*PI)*t;as = speed*(2*PI)*t;if (rs >mr)rs = mr;if (rs < 2)rs = 2;
                // rs = mr;
                h_injectorData[injNum][2][0] = ftToM(rs*sin(as));
                h_injectorData[injNum][2][2] = -ftToM(rs*cos(as))  ;

					
            }		
    	}	
    }
	
	
    if ((HOST == NEXCAVE) || (HOST == MAC_SIM_NEXCAVE))
    {
        int numobj = 5;
        h_injectorData[0][0][0] = numobj;

    	for(int i = 1;i<= numobj;i++)
    	{
            if (time_in_scene > 1.0/speed)reflect_on = 1;	
            if (time_in_scene > 1.5/speed)
            {
                reflect_on = 1;
			
                h_injectorData[injNum][3][0] = 0;
                h_injectorData[injNum][3][1] = .5;
                h_injectorData[injNum][3][2] = 0.0; // x,y,z velocity
                if (time_in_scene > 2.0/speed)
                {
                    h_injectorData[injNum][3][0] = -ftToM(rs*sin(as))/8 ;
                    h_injectorData[injNum][3][1] = .5;
                    h_injectorData[injNum][3][2] = ftToM(rs*cos(as))/8; // x,y,z velocity
            	    h_injectorData[injNum][6][0] = .026;
                    h_injectorData[injNum][6][1] = 0.0;
                    h_injectorData[injNum][6][2] = 0.0; // speed jitter ~ ~
                           
            	}
            }
    	 	
    	    copy_injector(1, i);
            stime = i*(1/speed/numobj);
            if (time_in_scene >stime)
            {
                injNum = i; t = time_in_scene - stime;rs = 2*speed*(2*PI)*t;as = speed*(2*PI)*t;if (rs >mr)rs = mr;if (rs < 2)rs = 2;
                // rs = mr;
                h_injectorData[injNum][2][0] = ftToM(rs*sin(as));
                h_injectorData[injNum][2][2] = ftToM(rs*cos(as)) - ftToM(5);

					
            }		
    	}	
    }
	

    h_reflectorData[0][0][0] = 2; // number of reflectors ~ ~   ~ means dont care
	
	
    int reflNum = 1;
    h_reflectorData[reflNum][0][0] = reflect_on;
    h_reflectorData[reflNum][0][1] = 0; // type, age ie colormod,, ~  0 is off 1 is plane reflector
    h_reflectorData[reflNum][1][0] = 0;
    h_reflectorData[reflNum][1][1] = 0.0;
    h_reflectorData[reflNum][1][2] = 0; // x,y,z position
    h_reflectorData[reflNum][2][0] = 0.0;
    h_reflectorData[reflNum][2][1] = 1;
    h_reflectorData[reflNum][2][2] = 0; // x,y,z normal
    h_reflectorData[reflNum][3][0] = ftToM(10.00);
    h_reflectorData[reflNum][3][1] = 0.00;
    h_reflectorData[reflNum][3][2] = 0; // reflector radis ,~,~ 
    h_reflectorData[reflNum][4][0] = 0.000;
    h_reflectorData[reflNum][4][1] = 0.000;
    h_reflectorData[reflNum][4][2] = 0.000; // t,u,v jitter  not implimented = speed 
    h_reflectorData[reflNum][5][0] = 0.4;
    h_reflectorData[reflNum][5][1] = 0.0;
    h_reflectorData[reflNum][5][2] = 0.0; // reflectiondamping , no_traping ~
    h_reflectorData[reflNum][6][0] = 0;
    h_reflectorData[reflNum][6][1] = 0;
    h_reflectorData[reflNum][6][2] = 0; // not implemented yet centrality of rnd distribution speed dt tu ~
	
    reflNum = 2;
// BOB XFORM THIS
    float x = wandPos[0];
    float y = wandPos[1];
    float z = wandPos[2];
    float dx = wandVec[0]/2;
    float dy = wandVec[1]/2;
    float dz = wandVec[2]/2;

    dx = wandMat[4];
    dy = wandMat[5];
    dz = wandMat[6];

    h_reflectorData[reflNum][0][0] = trigger;
    h_reflectorData[reflNum][0][1] = 1; // type, age ie colormod, ~  0 is off 1 is plane reflector  0 is off 1 is plane reflector
    h_reflectorData[reflNum][1][0] = x;
    h_reflectorData[reflNum][1][1] = y;
    h_reflectorData[reflNum][1][2] = z; // x,y,z position
    h_reflectorData[reflNum][2][0] = dx;
    h_reflectorData[reflNum][2][1] = dy;
    h_reflectorData[reflNum][2][2] = dz; // x,y,z normal
    h_reflectorData[reflNum][3][0] = ftToM(0.5);
    h_reflectorData[reflNum][3][1] = 0.00;
    h_reflectorData[reflNum][3][2] = 0; // reflector radis ,~,~ 
    h_reflectorData[reflNum][4][0] = 0.000;
    h_reflectorData[reflNum][4][1] = 0.000;
    h_reflectorData[reflNum][4][2] = 0.000; // t,u,v jitter  not implimented = speed 
    h_reflectorData[reflNum][5][0] = 1;
    h_reflectorData[reflNum][5][1] = 1.00;
    h_reflectorData[reflNum][5][2] = 0.0; // reflectiondamping , no_traping ~
    h_reflectorData[reflNum][6][0] = 0;
    h_reflectorData[reflNum][6][1] = 0;
    h_reflectorData[reflNum][6][2] = 0; // not implemented yet centrality of rnd distribution speed dt tu ~
     
    if ((SOUND_SERV == 1)&& (::host->root() == 1))
    {

        if ((REFL_HITS == 1 ) && (trigger))
        {
            float ag = h_debugData[reflNum]/100.0;
            if ((HOST == NEXCAVE) ||(HOST == MAC_SIM_NEXCAVE)) 
            {
                if (ag >.5) ag = .5;
            }
            else
            {
                if (ag >.5) ag = .5;
            }
            audioGain(dan_10122606_sound_spray,ag);
            // printf ("falling rain hits in scene %f  ln hits %f\n",h_debugData[reflNum],log((h_debugData[reflNum])));
        }
        if ((triggerold == 1) && (trigger == 0)) audioGain(dan_10122606_sound_spray,0);

    }

    scene1Start = 0;

}

void danpart::scene_data_2_kill_audio()
{
    h_reflectorData[0][0][0] = 0; // turn off all reflectors
    h_injectorData[0][0][0] = 0; // turn off all injectors

    if ((SOUND_SERV == 1)&& (::host->root() == 1))
    {
        audioFadeOutStop(harmonicAlgorithm, 150,-1);
        audioGain(dan_10122606_sound_spray,0);
    }
}

void danpart::scene_data_2()
{
// 4 waterFalls

    draw_water_sky = 0;
// particlesysparamiteres--------------
    gravity = .005;	
    gravity = .0001;	
    max_age = 2000;
    disappear_age = 2000;
    colorFreq = 64 *max_age/2000.0 ;
    alphaControl = 0; // turns alph to transparent beloy y = 0
    static float time_in_scene;
    time_in_scene = time_in_scene + 1.0/TARGET_FR_RATE;

// reload  rnd < max_age in to pdata

    if (scene2Start == 1)
    {
        if (DEBUG_PRINT >0)printf("in start scene2 \n");
        time_in_scene = 0 * TARGET_FR_RATE;
        ::user->home();
        if (DEBUG_PRINT >0)printf("scene0Start \n");
        if ((SOUND_SERV == 1)&& (::host->root() == 1))
        {
            // audioFadeUp(harmonicAlgorithm, 15, 1, -1);
            audioPlay(harmonicAlgorithm,1.0);audioGain(harmonicAlgorithm,1);
                    
        }
 
        // printf(" time %f \n", ::user->get_t());
    }


    if (time_in_scene >90)nextScene = 1;

    if (DEBUG_PRINT >0)printf("in scene2 \n");


    // anim += 0.001; // .0001
    static float rotRate = .05;
    anim = showFrameNo * rotRate;
    rotRate += .000001;

// 	 injector data 
    int injNum ;	
    h_injectorData[0][0][0] = 1; // number of injectors ~ ~   ~ means dont care

    if ((HOST == STARCAVE) || (HOST == MAC_SIM))
    {
 	
        h_injectorData[0][0][0] = 4; // number of injectors ~ ~   ~ means dont care

        // injector 1
        injNum = 1; // front
        h_injectorData[injNum][1][0] = 2;
        h_injectorData[injNum][1][1] = 1.0; // type, injection ratio ie streem volume, ~
        h_injectorData[injNum][2][0] = 0;
        h_injectorData[injNum][2][1] = ftToM(6);
        h_injectorData[injNum][2][2] = ftToM(-2); // x,y,z position
        h_injectorData[injNum][3][0] = 0.0;
        h_injectorData[injNum][3][1] = 0.010;
        h_injectorData[injNum][3][2] = 0.01; // x,y,z velocity drection
    	h_injectorData[injNum][4][0] = ftToM(0.25);
        h_injectorData[injNum][4][1] = ftToM(0.25);
        h_injectorData[injNum][4][2] = ftToM(0.0); // x,y,z size
        h_injectorData[injNum][5][0] = 0.000;
        h_injectorData[injNum][5][1] = 0.000;
        h_injectorData[injNum][5][2] = 0.000; // t,u,v jitter v not implimented = speed 
        h_injectorData[injNum][6][0] = 0.2000;
        h_injectorData[injNum][6][1] = 0.0;
        h_injectorData[injNum][6][2] = 0.0; // speed jitter ~ ~
        h_injectorData[injNum][7][0] = 5;
        h_injectorData[injNum][7][1] = 5;
        h_injectorData[injNum][7][2] = 5; // centrality of rnd distribution speed dt tu ~

        copy_injector(1, 2);
        injNum = 2; // back
    	h_injectorData[injNum][2][0] = 0;
        h_injectorData[injNum][2][1] = ftToM(6);
        h_injectorData[injNum][2][2] = ftToM(2); // x,y,z position
        h_injectorData[injNum][3][0] = 0.0;
        h_injectorData[injNum][3][1] = 0.010;
        h_injectorData[injNum][3][2] = -0.01; // x,y,z velocity drection

        copy_injector(1, 3);
    	injNum = 3; // back
        h_injectorData[injNum][2][0] = ftToM(2);
        h_injectorData[injNum][2][1] = ftToM(6);
        h_injectorData[injNum][2][2] = ftToM(0); // x,y,z position
        h_injectorData[injNum][3][0] = -0.010;
        h_injectorData[injNum][3][1] = 0.010;
        h_injectorData[injNum][3][2] = 0; // x,y,z velocity drection
    	h_injectorData[injNum][4][0] = ftToM(0.25);
        h_injectorData[injNum][4][1] = ftToM(0.25);
        h_injectorData[injNum][4][2] = ftToM(0.25); // x,y,z size

        copy_injector(1, 4);
        injNum = 4; // back
    	h_injectorData[injNum][2][0] = ftToM(-2);
        h_injectorData[injNum][2][1] = ftToM(6);
        h_injectorData[injNum][2][2] = ftToM(0); // x,y,z position
        h_injectorData[injNum][3][0] = 0.010;
        h_injectorData[injNum][3][1] = 0.010;
        h_injectorData[injNum][3][2] = 0; // x,y,z velocity drection
        h_injectorData[injNum][4][0] = ftToM(0.25);
        h_injectorData[injNum][4][1] = ftToM(0.25);
        h_injectorData[injNum][4][2] = ftToM(0.25); // x,y,z size
    }

    if ((HOST == NEXCAVE) || (HOST == MAC_SIM_NEXCAVE))
    {
 	
        h_injectorData[0][0][0] = 3; // number of injectors ~ ~   ~ means dont care

        // injector 1
        injNum = 1; // front
        h_injectorData[injNum][1][0] = 2;
        h_injectorData[injNum][1][1] = 1.0; // type, injection ratio ie streem volume, ~
        h_injectorData[injNum][2][0] = 0;
        h_injectorData[injNum][2][1] = ftToM(7);
        h_injectorData[injNum][2][2] = ftToM(-5); // x,y,z position
        h_injectorData[injNum][3][0] = 0.0;
        h_injectorData[injNum][3][1] = 0.010;
        h_injectorData[injNum][3][2] = 0.07; // x,y,z velocity drection
    	h_injectorData[injNum][4][0] = ftToM(0.25);
        h_injectorData[injNum][4][1] = ftToM(0.25);
        h_injectorData[injNum][4][2] = ftToM(0.0); // x,y,z size
        h_injectorData[injNum][5][0] = 0.000;
        h_injectorData[injNum][5][1] = 0.000;
        h_injectorData[injNum][5][2] = 0.000; // t,u,v jitter v not implimented = speed 
        h_injectorData[injNum][6][0] = 0.1000;
        h_injectorData[injNum][6][1] = 0.0;
        h_injectorData[injNum][6][2] = 0.0; // speed jitter ~ ~
        h_injectorData[injNum][7][0] = 5;
        h_injectorData[injNum][7][1] = 5;
        h_injectorData[injNum][7][2] = 5; // centrality of rnd distribution speed dt tu ~

        copy_injector(1, 2);
        injNum = 2; // back
    	h_injectorData[injNum][2][0] = ftToM(-2.5);
        h_injectorData[injNum][2][1] = ftToM(7);
        h_injectorData[injNum][2][2] = ftToM(-4); // x,y,z position
        h_injectorData[injNum][3][0] = 0.04;
        h_injectorData[injNum][3][1] = 0.010;
        h_injectorData[injNum][3][2] = 0.04; // x,y,z velocity drection

        copy_injector(1, 3);
    	injNum = 3; // back
        h_injectorData[injNum][2][0] = ftToM(2.6);
        h_injectorData[injNum][2][1] = ftToM(7);
        h_injectorData[injNum][2][2] = ftToM(-4); // x,y,z position
        h_injectorData[injNum][3][0] = -0.04;
        h_injectorData[injNum][3][1] = 0.010;
        h_injectorData[injNum][3][2] = 0.04; // x,y,z velocity drection
    	h_injectorData[injNum][4][0] = ftToM(0.25);
        h_injectorData[injNum][4][1] = ftToM(0.25);
        h_injectorData[injNum][4][2] = ftToM(0.25); // x,y,z size
        copy_injector(1, 4);
        injNum = 4; // back
    	h_injectorData[injNum][2][0] = ftToM(-2);
        h_injectorData[injNum][2][1] = ftToM(6);
        h_injectorData[injNum][2][2] = ftToM(0); // x,y,z position
        h_injectorData[injNum][3][0] = 0.010;
        h_injectorData[injNum][3][1] = 0.010;
        h_injectorData[injNum][3][2] = 0; // x,y,z velocity drection
        h_injectorData[injNum][4][0] = ftToM(0.25);
        h_injectorData[injNum][4][1] = ftToM(0.25);
        h_injectorData[injNum][4][2] = ftToM(0.25); // x,y,z size
    }



    h_reflectorData[0][0][0] = 1; // number of reflectors ~ ~   ~ means dont care
    int reflNum;
    reflNum = 1;
    float x = wandPos[0];
    float y = wandPos[1];
    float z = wandPos[2];
    float dx = wandVec[0]/2;
    float dy = wandVec[1]/2;
    float dz = wandVec[2]/2;

    dx = wandMat[4];
    dy = wandMat[5];
    dz = wandMat[6];


    h_reflectorData[reflNum][0][0] = trigger;
    h_reflectorData[reflNum][0][1] = 1; // type, age ie colormod, ~  0 is off 1 is plane reflector  0 is off 1 is plane reflector
    h_reflectorData[reflNum][1][0] = x;
    h_reflectorData[reflNum][1][1] = y;
    h_reflectorData[reflNum][1][2] = z; // x,y,z position
    h_reflectorData[reflNum][2][0] = dx;
    h_reflectorData[reflNum][2][1] = dy;
    h_reflectorData[reflNum][2][2] = dz; // x,y,z normal
    h_reflectorData[reflNum][3][0] = ftToM(0.5);
    h_reflectorData[reflNum][3][1] = 0.00;
    h_reflectorData[reflNum][3][2] = 0; // reflector radis ,~,~ 
    h_reflectorData[reflNum][4][0] = 0.000;
    h_reflectorData[reflNum][4][1] = 0.000;
    h_reflectorData[reflNum][4][2] = 0.000; // t,u,v jitter  not implimented = speed 
    h_reflectorData[reflNum][5][0] = 1;
    h_reflectorData[reflNum][5][1] = 1.00;
    h_reflectorData[reflNum][5][2] = 0.0; // reflectiondamping , no_traping ~
    h_reflectorData[reflNum][6][0] = 0;
    h_reflectorData[reflNum][6][1] = 0;
    h_reflectorData[reflNum][6][2] = 0; // not implemented yet centrality of rnd distribution speed dt tu ~
	    
    if ((SOUND_SERV == 1)&& (::host->root() == 1))
    {

        if ((REFL_HITS == 1 ) && (trigger))
        {
            float ag = h_debugData[reflNum]/100.0;
            if ((HOST == NEXCAVE) ||(HOST == MAC_SIM_NEXCAVE)) 
            {
                if (ag >.5) ag = 1.0;
            }
            else
            {
                if (ag >.5) ag = .5;
            }
            audioGain(dan_10122606_sound_spray,ag);
            // printf ("falling rain hits in scene %f  ln hits %f\n",h_debugData[reflNum],log((h_debugData[reflNum])));
        }
        if ((triggerold == 1) && (trigger == 0)) audioGain(dan_10122606_sound_spray,0);

    }


	
    scene2Start = 0;

}
// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /

void danpart::scene_data_3_kill_audio()
{
    h_reflectorData[0][0][0] = 0; // turn off all reflectors
    h_injectorData[0][0][0] = 0; // turn off all injectors

    if ((SOUND_SERV == 1)&& (::host->root() == 1))
    {
        audioFadeOutStop(rain_at_sea, 150,-1);
        audioFadeOutStop(texture_17_swirls3, 150,-1);
        audioGain(dan_10122606_sound_spray,0);
        audioGain(dan_10120600_rezS3_rez2,0);
    }
}
void danpart::scene_data_3()
{
// painting skys

    draw_water_sky = 1;
// particlesysparamiteres--------------
    gravity = .000005;	
    max_age = 2000;
    disappear_age = 2000;
    colorFreq = 64 *max_age/2000.0 ;
    alphaControl = 1; // turns alph to transparent beloy y = 0
    static float time_in_scene;
    static float rotRate;

    time_in_scene = time_in_scene + 1.0/TARGET_FR_RATE;

// reload  rnd < max_age in to pdata

    if (scene3Start == 1)
    {
        size_t size = PDATA_ROW_SIZE * CUDA_MESH_WIDTH * CUDA_MESH_HEIGHT * sizeof (float);
        pdata_init_age(max_age);
        pdata_init_velocity(-10000, -10000, -10000);
        pdata_init_rand();
        cuMemcpyHtoD(d_particleData, h_particleData, size);
        if (DEBUG_PRINT >0)printf("in start scene3 \n");
        time_in_scene = 0 * TARGET_FR_RATE;
        ::user->home();::user->turn(0,90,0);
        rotRate = .05;
        ::user->set_t(86400); // set to 00:00 monday  old
        ::user->set_t(1); // set to 0 days 1 sec
        ::user->set_t(63711); // set to 0 days 1 sec

        // ::user->time(43200/2.0); // set to dawn  old
        // ::user->set_t(43200/2.0);

        // ::user->pass(43200/2.0);
        // printf(" time %f \n", ::user->get_t());

        if ((SOUND_SERV == 1)&& (::host->root() == 1))
        {
            audioLoop(rain_at_sea,1);audioPlay(rain_at_sea,1.0);audioGain(rain_at_sea,1);
            audioLoop(texture_17_swirls3,1);audioPlay(texture_17_swirls3,1.0);audioGain(texture_17_swirls3,1);
                    
        }
    }
// printf ("time_in_scene 3 %f\n",time_in_scene);

    if (DEBUG_PRINT >0)printf("in scene3 \n");
// printf ("scenetime time %f %f\n",time_in_scene,::user->get_t());
// lerp(in, beginIN, endIn, beginOut, endOut)

    if((time_in_scene > 0)&& (time_in_scene <= 30)) user->set_t(lerp(time_in_scene, 0, 30, 63400, 74000));
    if ((HOST == NEXCAVE) || (HOST == MAC_SIM_NEXCAVE))
    {
        if((time_in_scene > 30)&& (time_in_scene <= 40)) user->set_t(lerp(time_in_scene, 30, 40, 74000, 90000));		
        if((time_in_scene > 40)&& (time_in_scene <= 90)) user->set_t(lerp(time_in_scene, 30, 90, 90000, 63400));
        if((time_in_scene > 40)) ::user->move(0,.005,0);	
    }
    else
    {
        if((time_in_scene > 30)&& (time_in_scene <= 90)) user->set_t(lerp(time_in_scene, 30, 90, 74000, 107000));
    }
// if((time_in_scene > 30)&& (time_in_scene <= 40)) user->set_t(lerp(time_in_scene, 30, 40, 74000, 110000));
    //     ::user->set_t(107000); // set to 0 days 1 sec


    // anim += 0.001; // .0001

    anim = showFrameNo * rotRate;
    rotRate += .000001;

    // anim += 3.14159/4;
    // tracker data
    // printf("  devid %d \n",devid );
    // printf("pos  P %f %f %f\n", P[0], P[1], P[2]);
    // printf(" direc  V %f %f %f\n", V[0], V[1], V[2]);


// 	 injector data 

// 	 injector data 
    int injNum ;	
    h_injectorData[0][0][0] = 3; // number of injectors ~ ~   ~ means dont care
    // injector 1
    injNum = 1;
	
    h_injectorData[injNum][1][0] = 1;
    h_injectorData[injNum][1][1] = 1.0; // type, injection ratio ie streem volume, ~
    h_injectorData[injNum][2][0] = 0;
    h_injectorData[injNum][2][1] = ftToM(8.00);
    h_injectorData[injNum][2][2] = 0; // x,y,z position
    h_injectorData[injNum][3][0] = 1 * (sin(anim/5));
    h_injectorData[injNum][3][1] = 0.000;
    h_injectorData[injNum][3][2] = 1 * (cos(anim/5)); // x,y,z velocity direction
    h_injectorData[injNum][3][0] = 0.02 * (sin(anim));
    h_injectorData[injNum][3][1] = 0;
    h_injectorData[injNum][3][2] = 0.02 * (cos(anim)); // x,y,z velocity
	
    h_injectorData[injNum][4][0] = 0.00;
    h_injectorData[injNum][4][1] = 0.00;
    h_injectorData[injNum][4][2] = .0; // x,y,z size
    h_injectorData[injNum][5][0] = 0.0010;
    h_injectorData[injNum][5][1] = 0.0010;
    h_injectorData[injNum][5][2] = 0.000; // t,u,v jitter v not implimented = speed 
    h_injectorData[injNum][6][0] = 1.1;
    h_injectorData[injNum][6][1] = 0.0;
    h_injectorData[injNum][6][2] = 0.0; // speed jitter ~ ~
    h_injectorData[injNum][7][0] = 5;
    h_injectorData[injNum][7][1] = 5;
    h_injectorData[injNum][7][2] = 5; // centrality of rnd distribution speed dt tu ~

    if ((SOUND_SERV == 1)&& (::host->root() == 1))
    {
        if (ENABLE_SOUND_POS_UPDATES) audioPos (texture_17_swirls3, 30* h_injectorData[injNum][3][0], 0, -30* h_injectorData[injNum][3][2]);
			
    }

    // 
    // injector 2
    injNum = 2;
    h_injectorData[injNum][1][0] = 1;
    h_injectorData[injNum][1][1] = 0.0; // type, injection ratio ie streem volume, ~
    h_injectorData[injNum][2][0] = 0;
    h_injectorData[injNum][2][1] = 2.5;
    h_injectorData[injNum][2][2] = 0; // x,y,z position
    h_injectorData[injNum][3][0] = 0.0;
    h_injectorData[injNum][3][1] = 1.000;
    h_injectorData[injNum][3][2] = 0; // x,y,z velocity drection
    h_injectorData[injNum][4][0] = 0.25;
    h_injectorData[injNum][4][1] = 0.25;
    h_injectorData[injNum][4][2] = 0; // x,y,z size
    h_injectorData[injNum][5][0] = 0.000;
    h_injectorData[injNum][5][1] = 0.000;
    h_injectorData[injNum][5][2] = 0.000; // t,u,v jitter v not implimented = speed 
    h_injectorData[injNum][6][0] = .00;
    h_injectorData[injNum][6][1] = 0.5;
    h_injectorData[injNum][6][2] = 0.0; // speed jitter ~ ~
    h_injectorData[injNum][7][0] = 5;
    h_injectorData[injNum][7][1] = 5;
    h_injectorData[injNum][7][2] = 5; // centrality of rnd distribution speed dt tu ~

    // injector 3
    // sound for inj3
    injNum = 3;

    if ((SOUND_SERV == 1)&& (::host->root() == 1))
    {
        // audioGain(texture_12,trigger);
	
        // if ((triggerold == 0) && (trigger == 1)){audioPlay(short_sound_01a,0.1);audioGain(texture_12,1);}
            
        if ((triggerold == 0) && (trigger == 1)){audioGain(short_sound_01a,10.0);audioFadeUp(texture_12, 1, 1, short_sound_01a);}
        if ((triggerold == 1) && (trigger == 0)){audioFadeOut(texture_12, 10, -1);}
        
        if (ENABLE_SOUND_POS_UPDATES)
        {  
            audioPos (texture_12, 1* h_injectorData[injNum][2][0], 1* h_injectorData[injNum][1][0], -1* h_injectorData[injNum][2][2]);
            audioPos (short_sound_01a, 1* h_injectorData[injNum][2][0], 1* h_injectorData[injNum][1][0], -1* h_injectorData[injNum][2][2]);
        } 

    }
    h_injectorData[injNum][1][0] = 1;
    h_injectorData[injNum][1][1] = trigger*4.0; // type, injection ratio ie streem volume, ~
    h_injectorData[injNum][2][0] = wandPos[0];
    h_injectorData[injNum][2][1] = wandPos[1];
    h_injectorData[injNum][2][2] = wandPos[2]; // x,y,z position
    h_injectorData[injNum][3][0] = wandVec[0];
    h_injectorData[injNum][3][1] = wandVec[1];
    h_injectorData[injNum][3][2] = wandVec[2]; // x,y,z velocity direction
    h_injectorData[injNum][4][0] = 0.00;
    h_injectorData[injNum][4][1] = 0.00;
    h_injectorData[injNum][4][2] = .0; // x,y,z size
    h_injectorData[injNum][5][0] = 0.010;
    h_injectorData[injNum][5][1] = 0.010;
    h_injectorData[injNum][5][2] = 0.000; // t,u,v jitter v not implimented = speed 
    h_injectorData[injNum][6][0] = 0.1;
    h_injectorData[injNum][6][1] = 0.0;
    h_injectorData[injNum][6][2] = 0.0; // speed jitter ~ ~
    h_injectorData[injNum][7][0] = 5;
    h_injectorData[injNum][7][1] = 5;
    h_injectorData[injNum][7][2] = 5; // centrality of rnd distribution speed dt tu ~
    // 
    if (trigger){if (DEBUG_PRINT >0) {printf(" wandPos[0 ,1,2] wandVec[0,1,2] %f %f %f    %f %f %f \n", wandPos[0],wandPos[1],wandPos[2],wandVec[0],wandVec[1],wandVec[2]);}}
   
    scene3Start = 0;
}

//-----------------------------------------------------------------------------

void danpart::scene_data_4_kill_audio()
{
    h_reflectorData[0][0][0] = 0; // turn off all reflectors
    h_injectorData[0][0][0] = 0; // turn off all injectors

    if ((SOUND_SERV == 1)&& (::host->root() == 1))
    {

        audioFadeOutStop(dan_rain_at_sea_loop, 150,-1);
        audioGain(dan_10122606_sound_spray,0);
    }
}

void danpart::scene_data_4()
{
    // falling rain

    draw_water_sky = 0;

    // particle system parameters

    gravity = .001;	
    gravity = .00003;	
    max_age = 2000;
    disappear_age = 2000;
    colorFreq = 64 *max_age/2000.0 ;
    alphaControl = 0; // turns alph to transparent beloy y = 0
    static float time_in_scene;
    time_in_scene = time_in_scene + 1.0/TARGET_FR_RATE;

    // reload  rnd < max_age in to pdata

    if (scene4Start == 1)
    {
        ::user->home();

        if (DEBUG_PRINT > 0) printf("in start scene4 \n");

        time_in_scene = 0 * TARGET_FR_RATE;

        if ((SOUND_SERV == 1) && (::host->root() == 1))
        {
            audioLoop(dan_rain_at_sea_loop, 1);
            audioPlay(dan_rain_at_sea_loop, 1.0);
            audioGain(dan_rain_at_sea_loop, 1.0);
        }
    }

    if (DEBUG_PRINT >0) printf("in scene4 \n");

    static float rotRate = .05;
    anim = showFrameNo * rotRate;
    rotRate += .000001;

    // injector data 
    h_reflectorData[0][0][0] = 1; // turn off all reflectors

    // injector data 
    int injNum ;	
    h_injectorData[0][0][0] = 1; // number of injectors ~ ~   ~ means dont care

    // injector 1
    if ((HOST == STARCAVE) || (HOST == MAC_SIM))
    {
        injNum = 1;
        h_injectorData[injNum][1][0] = 2.0;
        h_injectorData[injNum][1][1] = 1.0;       // type, injection ratio ie streem volume, ~
        h_injectorData[injNum][2][0] = ftToM( 0);
        h_injectorData[injNum][2][1] = ftToM(10);
        h_injectorData[injNum][2][2] = ftToM(-2); // x,y,z position
        h_injectorData[injNum][3][0] = 0.0;
        h_injectorData[injNum][3][1] = 0.001;
        h_injectorData[injNum][3][2] = 0.0;       // x,y,z velocity drection
        h_injectorData[injNum][4][0] = ftToM(1);
        h_injectorData[injNum][4][1] = ftToM(1);
        h_injectorData[injNum][4][2] = ftToM(1);  // x,y,z size
        h_injectorData[injNum][5][0] = 0.000;
        h_injectorData[injNum][5][1] = 0.000;
        h_injectorData[injNum][5][2] = 0.000;     // t,u,v jitter v not implimented = speed 
        h_injectorData[injNum][6][0] = 0.2000;
        h_injectorData[injNum][6][1] = 0.0;
        h_injectorData[injNum][6][2] = 0.0;       // speed jitter
        h_injectorData[injNum][7][0] = 5.0;
        h_injectorData[injNum][7][1] = 5.0;
        h_injectorData[injNum][7][2] = 5.0;       // centrality of rnd distribution speed dt tu
    }
  	
    if ((HOST == NEXCAVE) || (HOST == MAC_SIM_NEXCAVE))
    { 	  
    	injNum = 1;
        h_injectorData[injNum][1][0] = 2;
        h_injectorData[injNum][1][1] = 1.0; // type, injection ratio ie streem volume, ~
    	h_injectorData[injNum][2][0] = 0;
        h_injectorData[injNum][2][1] = ftToM( 7);
        h_injectorData[injNum][2][2] = ftToM(-3); // x,y,z position
    	h_injectorData[injNum][3][0] = 0.0;
        h_injectorData[injNum][3][1] = -0.001;
        h_injectorData[injNum][3][2] = 0.00; // x,y,z velocity drection
    	h_injectorData[injNum][4][0] = ftToM(1.0);
        h_injectorData[injNum][4][1] = ftToM(0.5);
        h_injectorData[injNum][4][2] = ftToM(1.0); // x,y,z size
    	h_injectorData[injNum][5][0] = 0.000;
        h_injectorData[injNum][5][1] = 0.000;
        h_injectorData[injNum][5][2] = 0.000; // t,u,v jitter v not implimented = speed 
    	h_injectorData[injNum][6][0] = 0.2000;
        h_injectorData[injNum][6][1] = 0.0;
        h_injectorData[injNum][6][2] = 0.0; // speed jitter ~ ~
    	h_injectorData[injNum][7][0] = 5.0;
        h_injectorData[injNum][7][1] = 5.0;
        h_injectorData[injNum][7][2] = 5.0; // centrality of rnd distribution speed dt tu ~
    }

    copy_injector(1, 2);
    injNum = 2; // back
    h_injectorData[injNum][2][0] = 0;
    h_injectorData[injNum][2][1] = ftToM(6);
    h_injectorData[injNum][2][2] = ftToM(2); // x,y,z position
    h_injectorData[injNum][3][0] = 0.0;
    h_injectorData[injNum][3][1] = 0.010;
    h_injectorData[injNum][3][2] = -0.01; // x,y,z velocity drection

    copy_injector(1, 3);
    injNum = 3; // back
    h_injectorData[injNum][2][0] = ftToM(2);
    h_injectorData[injNum][2][1] = ftToM(6);
    h_injectorData[injNum][2][2] = ftToM(0); // x,y,z position
    h_injectorData[injNum][3][0] = -0.010;
    h_injectorData[injNum][3][1] = 0.010;
    h_injectorData[injNum][3][2] = 0; // x,y,z velocity drection
    h_injectorData[injNum][4][0] = ftToM(0.25);
    h_injectorData[injNum][4][1] = ftToM(0.25);
    h_injectorData[injNum][4][2] = ftToM(0.25); // x,y,z size

    copy_injector(1, 4);
    injNum = 4; // back
    h_injectorData[injNum][2][0] = ftToM(-2);
    h_injectorData[injNum][2][1] = ftToM(6);
    h_injectorData[injNum][2][2] = ftToM(0); // x,y,z position
    h_injectorData[injNum][3][0] = 0.010;
    h_injectorData[injNum][3][1] = 0.010;
    h_injectorData[injNum][3][2] = 0; // x,y,z velocity drection
    h_injectorData[injNum][4][0] = ftToM(0.25);
    h_injectorData[injNum][4][1] = ftToM(0.25);
    h_injectorData[injNum][4][2] = ftToM(0.25); // x,y,z size

    h_reflectorData[0][0][0] = 1; // number of reflectors ~ ~   ~ means dont care
    int reflNum;
    reflNum = 1;
    float x = wandPos[0];
    float y = wandPos[1];
    float z = wandPos[2];
    float dx = wandVec[0]/2;
    float dy = wandVec[1]/2;
    float dz = wandVec[2]/2;

    dx = wandMat[4];
    dy = wandMat[5];
    dz = wandMat[6];

    h_reflectorData[reflNum][0][0] = trigger;
    h_reflectorData[reflNum][0][1] = 1; // type, age ie colormod, ~  0 is off 1 is plane reflector  0 is off 1 is plane reflector
    h_reflectorData[reflNum][1][0] = x;
    h_reflectorData[reflNum][1][1] = y;
    h_reflectorData[reflNum][1][2] = z; // x,y,z position
    h_reflectorData[reflNum][2][0] = dx;
    h_reflectorData[reflNum][2][1] = dy;
    h_reflectorData[reflNum][2][2] = dz; // x,y,z normal
    h_reflectorData[reflNum][3][0] = ftToM(0.5);
    h_reflectorData[reflNum][3][1] = 0.00;
    h_reflectorData[reflNum][3][2] = 0; // reflector radis ,~,~ 
    h_reflectorData[reflNum][4][0] = 0.000;
    h_reflectorData[reflNum][4][1] = 0.000;
    h_reflectorData[reflNum][4][2] = 0.000; // t,u,v jitter  not implimented = speed 
    h_reflectorData[reflNum][5][0] = 1;
    h_reflectorData[reflNum][5][1] = 1.00;
    h_reflectorData[reflNum][5][2] = 0.0; // reflectiondamping , no_traping ~
    h_reflectorData[reflNum][6][0] = 0;
    h_reflectorData[reflNum][6][1] = 0;
    h_reflectorData[reflNum][6][2] = 0; // not implemented yet centrality of rnd distribution speed dt tu ~
	    
    if ((SOUND_SERV == 1) && (::host->root() == 1))
    {
        if ((REFL_HITS == 1) && (trigger))
        {
            float ag = h_debugData[reflNum] / 500.0;

            if ((HOST == NEXCAVE) || (HOST == MAC_SIM_NEXCAVE)) 
            {
                if (ag > .5) ag = .5;
            }
            else
            {
                if (ag > .5) ag = .5;
            }
            audioGain(dan_10122606_sound_spray, ag);
        }
        if ((triggerold == 1) && (trigger == 0))
            audioGain(dan_10122606_sound_spray, 0);
    }

    scene4Start = 0;
}

//-----------------------------------------------------------------------------
