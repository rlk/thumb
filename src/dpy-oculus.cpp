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

#include <cassert>

#include <etc-math.hpp>
#include <app-glob.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <ogl-program.hpp>
#include <dpy-channel.hpp>
#include <dpy-oculus.hpp>

//-----------------------------------------------------------------------------

static void getMatrix4f(const OVR::Matrix4f& src, double *dst)
{
    dst[ 0] = double(src.M[0][0]);
    dst[ 1] = double(src.M[1][0]);
    dst[ 2] = double(src.M[2][0]);
    dst[ 3] = double(src.M[3][0]);
    dst[ 4] = double(src.M[0][1]);
    dst[ 5] = double(src.M[1][1]);
    dst[ 6] = double(src.M[2][1]);
    dst[ 7] = double(src.M[3][1]);
    dst[ 8] = double(src.M[0][2]);
    dst[ 9] = double(src.M[1][2]);
    dst[10] = double(src.M[2][2]);
    dst[11] = double(src.M[3][2]);
    dst[12] = double(src.M[0][3]);
    dst[13] = double(src.M[1][3]);
    dst[14] = double(src.M[2][3]);
    dst[15] = double(src.M[3][3]);
}

OVR::Ptr<OVR::DeviceManager>    dpy::oculus::pManager;
OVR::Ptr<OVR::HMDDevice>        dpy::oculus::pHMD;
OVR::Ptr<OVR::SensorDevice>     dpy::oculus::pSensor;

OVR::HMDInfo                    dpy::oculus::Info;
OVR::SensorFusion               dpy::oculus::Fusion;
OVR::Util::Render::StereoConfig dpy::oculus::Stereo;

//-----------------------------------------------------------------------------

dpy::oculus::oculus(app::node p) :
    display(p), frust(0), chani(0), program(0)
{
    using namespace OVR::Util::Render;

    // Instantiate a view frustum object for later use in view culling.

    frust = new app::frustum(0, viewport[2], viewport[3]);
    chani = p.get_i("channel");

    // Initialize LibOVR if not already done.

    if (!OVR::System::IsInitialized())
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

        // Initialize OVR, the device, the sensor, and the sensor fusion.

        OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));

        if ((pManager = *OVR::DeviceManager::Create()))
        {
            if ((pHMD = *pManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice()))
            {
                if ((pSensor = *pHMD->GetSensor()))
                {
                    Fusion.AttachToSensor(pSensor);
                    pHMD->GetDeviceInfo(&Info);

                    Stereo.SetHMDInfo(Info);
                    Stereo.SetDistortionFitPointVP(-1.0f, 0.0f);
                    Stereo.SetStereoMode(Stereo_LeftRight_Multipass);
                    Stereo.SetFullViewport(Viewport(0, 0, Info.HResolution,
                                                          Info.VResolution));
                }
            }
        }
    }

    // Apply the Oculus projections to the frustums.

    if (OVR::System::IsInitialized())
    {
        double P[16];

        if (chani)
            getMatrix4f(Stereo.GetEyeRenderParams(StereoEye_Right).Projection, P);
        else
            getMatrix4f(Stereo.GetEyeRenderParams(StereoEye_Left).Projection,  P);

        frust->set_projection(P);
    }
}

dpy::oculus::~oculus()
{
    if (OVR::System::IsInitialized())
    {
        pSensor  = 0;
        pHMD     = 0;
        pManager = 0;

        OVR::System::Destroy();
    }

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
    if (chani < chanc)
        frust->set_viewpoint(chanv[chani]->get_p());
}

void dpy::oculus::draw(int chanc, const dpy::channel * const *chanv, int frusi)
{
    double center = 1.0 - (2.0 * Info.LensSeparationDistance)
                               / Info.HScreenSize;
    if (chani < chanc)
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
                int w = chanv[chani]->get_w();
                int h = chanv[chani]->get_h();

                program->uniform("ImageSize", double(w), double(h));

                if (chani)
                    program->uniform("LensCenter", 0.5 - 0.5 * center, 0.5);
                else
                    program->uniform("LensCenter", 0.5 + 0.5 * center, 0.5);

                fill(frust->get_w(),
                     frust->get_h(), w, h);
            }
            program->free();
        }
        chanv[chani]->free_color(GL_TEXTURE0);
    }
}

void dpy::oculus::test(int chanc, const dpy::channel *const *chanv, int index)
{
}

//-----------------------------------------------------------------------------

bool dpy::oculus::pointer_to_3D(app::event *E, int x, int y)
{
    assert(frust);

    // Determine whether the pointer falls within the viewport.

    if (viewport[0] <= x && x < viewport[0] + viewport[2] &&
        viewport[1] <= y && y < viewport[1] + viewport[3])

        // Let the frustum project the pointer into space.

        return frust->pointer_to_3D(E, x - viewport[0],
                         viewport[3] - y + viewport[1]);
    else
        return false;
}

bool dpy::oculus::process_start(app::event *E)
{
    // Initialize the shader.

    if ((program = ::glob->load_program("dpy/oculus.xml")))
    {
        double scale  = Stereo.GetDistortionScale();
        double aspect = double(Info.HResolution)
                      / double(Info.VResolution) / 2;

        program->uniform("DistortionK",        Info.DistortionK[0],
                                               Info.DistortionK[1],
                                               Info.DistortionK[2],
                                               Info.DistortionK[3]);
        program->uniform("ChromaAbCorrection", Info.ChromaAbCorrection[0],
                                               Info.ChromaAbCorrection[1],
                                               Info.ChromaAbCorrection[2],
                                               Info.ChromaAbCorrection[3]);

        program->uniform("ScaleOut", 0.5 / scale, 0.5 * aspect / scale);
        program->uniform("ScaleIn",  2.0,         2.0 / aspect);
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
