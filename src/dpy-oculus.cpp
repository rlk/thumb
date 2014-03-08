//  Copyright (C) 2007-2011 Robert Kooima
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

#ifdef WITH_OCULUS

#include <cassert>

#include <etc-vector.hpp>
#include <app-glob.hpp>
#include <app-host.hpp>
#include <app-view.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <ogl-program.hpp>
#include <dpy-channel.hpp>
#include <dpy-oculus.hpp>

//-----------------------------------------------------------------------------

namespace dpy
{
    class oculus_api
    {
    public:

        oculus_api()
        {
            OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));
        }
       ~oculus_api()
        {
            OVR::System::Destroy();
        }
    };

    class oculus_dev
    {
    public:

        oculus_api api;

        OVR::Ptr<OVR::DeviceManager> pManager;
        OVR::Ptr<OVR::HMDDevice>     pHMD;
        OVR::Ptr<OVR::SensorDevice>  pSensor;

        OVR::SensorFusion Fusion;
        OVR::HMDInfo      Info;

        oculus_dev() : api()
        {
            // Set default HMD info for a 7" OVR DK1 in case OVR fails.

            Info.DesktopX               =  0;
            Info.DesktopY               =  0;
            Info.HResolution            =  1280;
            Info.VResolution            =  800;

            Info.HScreenSize            =  0.14976f;
            Info.VScreenSize            =  0.09350f;
            Info.InterpupillaryDistance =  0.0604f;
            Info.LensSeparationDistance =  0.0635f;
            Info.EyeToScreenDistance    =  0.0410f;
            Info.VScreenCenter          =  Info.VScreenSize * 0.5f;

            Info.DistortionK[0]         =  1.00f;
            Info.DistortionK[1]         =  0.22f;
            Info.DistortionK[2]         =  0.24f;
            Info.DistortionK[3]         =  0.00f;

            Info.ChromaAbCorrection[0]  =  0.996f;
            Info.ChromaAbCorrection[1]  = -0.004f;
            Info.ChromaAbCorrection[2]  =  1.014f;
            Info.ChromaAbCorrection[3]  =  0.000f;

            // Initialize the HMD and sensor.

            if ((pManager = *OVR::DeviceManager::Create()))
            {
                if ((pHMD = *pManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice()))
                {
                    if ((pSensor = *pHMD->GetSensor()))
                    {
                        Fusion.AttachToSensor(pSensor);
                    }
                    pHMD->GetDeviceInfo(&Info);
                }
            }
        }
       ~oculus_dev()
        {
            Fusion.AttachToSensor(0);

            pSensor  = 0;
            pHMD     = 0;
            pManager = 0;
        }

    };

    // Statically-defined device that will RAII the Oculus API and HMD into
    // a functional state and clean them it upon exit.

    static oculus_dev device;
}

//-----------------------------------------------------------------------------

static mat4 getMatrix4f(const OVR::Matrix4f& src)
{
    return mat4(double(src.M[0][0]),
                double(src.M[0][1]),
                double(src.M[0][2]),
                double(src.M[0][3]),
                double(src.M[1][0]),
                double(src.M[1][1]),
                double(src.M[1][2]),
                double(src.M[1][3]),
                double(src.M[2][0]),
                double(src.M[2][1]),
                double(src.M[2][2]),
                double(src.M[2][3]),
                double(src.M[3][0]),
                double(src.M[3][1]),
                double(src.M[3][2]),
                double(src.M[3][3]));
}

//-----------------------------------------------------------------------------

dpy::oculus::oculus(app::node p) :
    display(p), frust(0), chani(0), program(0)
{
    using namespace OVR::Util::Render;

    chani = p.get_i("channel");

    // Apply the Oculus projections to the frustums.

    if (OVR::System::IsInitialized())
    {
        mat4 P;

        Stereo.SetHMDInfo(device.Info);
        Stereo.SetDistortionFitPointVP(-1.0f, 0.0f);
        Stereo.SetStereoMode(Stereo_LeftRight_Multipass);
        Stereo.SetFullViewport(Viewport(0, 0, device.Info.HResolution,
                                              device.Info.VResolution));
        if (chani)
            P = getMatrix4f(Stereo.GetEyeRenderParams(StereoEye_Right).Projection);
        else
            P = getMatrix4f(Stereo.GetEyeRenderParams(StereoEye_Left).Projection);

        frust = new app::perspective_frustum(P);
    }
    else
        frust = new app::calibrated_frustum();
}

dpy::oculus::~oculus()
{
    ::view->set_tracking(mat4());

    delete frust;
}

//-----------------------------------------------------------------------------

int dpy::oculus::get_frusc() const
{
    return 1;
}

void dpy::oculus::get_frusv(app::frustum **frusv) const
{
    frusv[0] = frust;
}

//-----------------------------------------------------------------------------

void dpy::oculus::prep(int chanc, const dpy::channel *const *chanv)
{
    using namespace OVR::Util::Render;

    mat4 T;

    // Include the Oculus eye offset in the tracking matrix.

    if (chani)
        T = getMatrix4f(Stereo.GetEyeRenderParams(StereoEye_Right).ViewAdjust);
    else
        T = getMatrix4f(Stereo.GetEyeRenderParams(StereoEye_Left).ViewAdjust);

    // Include the Oculus orientation in the tracking matrix.

    if (device.pSensor)
    {
        OVR::Vector3f v;
        float         a;

        device.Fusion.GetOrientation().GetAxisAngle(&v, &a);
        T = T * translation(vec3(0, -0.1, 0))
              *    rotation(vec3(double(v.x),
                                 double(v.y),
                                 double(v.z)), double(a))
              * translation(vec3(0, +0.1, 0));
    }

    // Set the tracking matrix.

    ::view->set_tracking(T);
}

void dpy::oculus::draw(int chanc, const dpy::channel * const *chanv, int frusi)
{
    assert(chanv[chani]);
    assert(program);

    // Draw the scene to the off-screen buffer.

    chanv[chani]->bind();
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ::host->draw(frusi, frust, chani);
    }
    chanv[chani]->free();

    // Draw the off-screen buffer to the screen.

    chanv[chani]->bind_color(GL_TEXTURE0);
    {
        program->bind();
        {
            int w = chanv[chani]->get_width();
            int h = chanv[chani]->get_height();

            program->uniform("LensCenter", LensCenter);
            program->uniform("ImageSize",  vec2(w, h));

            fill(frust->get_width(),
                 frust->get_height(), w, h);
        }
        program->free();
    }
    chanv[chani]->free_color(GL_TEXTURE0);
}

void dpy::oculus::test(int chanc, const dpy::channel *const *chanv, int index)
{
    assert(chanv[chani]);
    assert(program);

    // Draw the test pattern to the off-screen buffer.

    chanv[chani]->bind();
    {
        chanv[chani]->test();
    }
    chanv[chani]->free();

    // Draw the off-screen buffer to the screen.

    chanv[chani]->bind_color(GL_TEXTURE0);
    {
        program->bind();
        {
            int w = chanv[chani]->get_width();
            int h = chanv[chani]->get_height();

            program->uniform("LensCenter", LensCenter);
            program->uniform("ImageSize",  vec2(w, h));

            fill(frust->get_width(),
                 frust->get_height(), w, h);
        }
        program->free();
    }
    chanv[chani]->free_color(GL_TEXTURE0);
}

//-----------------------------------------------------------------------------

bool dpy::oculus::pointer_to_3D(app::event *E, int x, int y)
{
    assert(frust);

    // Determine whether the pointer falls within the viewport.

    double s = double(x - viewport[0]) / viewport[2];
    double t = double(y - viewport[1]) / viewport[3];

    if (true) // HACK 0.0 <= s && s < 1.0 && 0.0 <= t && t < 1.0)
    {
        // Apply the lens distortion to the pointer position.

        double vx = (s - LensCenter[0]) * ScaleIn[0];
        double vy = (t - LensCenter[1]) * ScaleIn[1];

        double rr = vx * vx + vy * vy;

        double k = (DistortionK[0] +
                    DistortionK[1] * rr +
                    DistortionK[2] * rr * rr +
                    DistortionK[3] * rr * rr * rr);

        double ox = LensCenter[0] + ScaleOut[0] * vx * k;
        double oy = LensCenter[1] + ScaleOut[1] * vy * k;

        // Let the frustum project the pointer into space.

        return frust->pointer_to_3D(E, ox, oy);
    }
    else
        return false;
}

bool dpy::oculus::process_start(app::event *E)
{
    // Initialize the shader.

    if ((program = ::glob->load_program("dpy/oculus.xml")))
    {
        double scale  = Stereo.GetDistortionScale();
        double aspect = double(device.Info.HResolution)
                      / double(device.Info.VResolution) / 2;

        double center = 1 - (2 * double(device.Info.LensSeparationDistance))
                               / double(device.Info.HScreenSize);

        if (chani) LensCenter = vec2(0.5 - 0.5 * center, 0.5);
        else       LensCenter = vec2(0.5 + 0.5 * center, 0.5);

        DistortionK =        vec4(device.Info.DistortionK[0],
                                  device.Info.DistortionK[1],
                                  device.Info.DistortionK[2],
                                  device.Info.DistortionK[3]);
        ChromaAbCorrection = vec4(device.Info.ChromaAbCorrection[0],
                                  device.Info.ChromaAbCorrection[1],
                                  device.Info.ChromaAbCorrection[2],
                                  device.Info.ChromaAbCorrection[3]);

        ScaleOut = vec2(0.5 / scale, 0.5 * aspect / scale);
        ScaleIn =  vec2(2.0,         2.0 / aspect);

        program->uniform("DistortionK",        DistortionK);
        program->uniform("ChromaAbCorrection", ChromaAbCorrection);
        program->uniform("ScaleOut",           ScaleOut);
        program->uniform("ScaleIn",            ScaleIn);
    }
    return false;
}

bool dpy::oculus::process_close(app::event *E)
{
    // Finalize the shader.

    ::glob->free_program(program);

    program = 0;
    return false;
}

bool dpy::oculus::process_event(app::event *E)
{
    // Do the local startup or shutdown.

    switch (E->get_type())
    {
    case E_START: process_start(E); break;
    case E_CLOSE: process_close(E); break;
    }
    return false;
}

//-----------------------------------------------------------------------------

#endif
