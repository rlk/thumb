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

#ifdef CONFIG_OCULUS

#include <cassert>

#include <etc-vector.hpp>
#include <etc-log.hpp>
#include <app-glob.hpp>
#include <app-host.hpp>
#include <app-view.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <ogl-program.hpp>
#include <dpy-channel.hpp>
#include <dpy-oculus.hpp>

//-----------------------------------------------------------------------------

/// Convert an OVR matrix to a thumb matrix.

static mat4 getMatrix4f(const OVR::Matrix4f& m)
{
    return mat4(m.M[0][0], m.M[0][1], m.M[0][2], m.M[0][3],
                m.M[1][0], m.M[1][1], m.M[1][2], m.M[1][3],
                m.M[2][0], m.M[2][1], m.M[2][2], m.M[2][3],
                m.M[3][0], m.M[3][1], m.M[3][2], m.M[3][3]);
}

//-----------------------------------------------------------------------------

dpy::oculus::oculus(app::node p, int window_rect[4], int buffer_size[2])
    : display(p), hmd(0), setup(false)
{
    ovr_Initialize();

    // Use the first connected HMD.

    hmd = ovrHmd_Create(0);

    // Fall back on a DK1 debug configuration if no HMD is available.

    if (hmd == 0)
        hmd = ovrHmd_CreateDebug(ovrHmd_DK1);
    
    if (hmd)
    {
        // Enable all tracking capabilities on this HMD.

        ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation |
                                      ovrTrackingCap_MagYawCorrection |
                                      ovrTrackingCap_Position, 0);

        // Override the window configuration .
        
        window_rect[0] = hmd->WindowsPos.x;
        window_rect[1] = hmd->WindowsPos.y;
        window_rect[2] = hmd->Resolution.w;
        window_rect[3] = hmd->Resolution.h;

        // Determine the buffer size required by each eye of the current HMD.

        ovrSizei sz0 = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left,
                                                hmd->DefaultEyeFov[0], 1);
        ovrSizei sz1 = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right,
                                                hmd->DefaultEyeFov[1], 1);

        buffer_size[0] = std::max(sz0.w, sz1.w);
        buffer_size[1] = std::max(sz0.h, sz1.h);
    }
    memset(tex, 0, 2 * sizeof (ovrTexture));
}

dpy::oculus::~oculus()
{
    if (hmd) ovrHmd_Destroy(hmd);

    ovr_Shutdown();
}

//-----------------------------------------------------------------------------

int dpy::oculus::get_frusc() const
{
    return hmd ? 2 : 0;
}

void dpy::oculus::get_frusv(app::frustum **frusv) const
{
    if (hmd)
    {
        frusv[0] = frust[0];
        frusv[1] = frust[1];
    }
}

//-----------------------------------------------------------------------------

void dpy::oculus::prep(int chanc, const dpy::channel *const *chanv)
{
    // GetEyePoses. Set each frustum to PV. Tracking is unused.

    // Copy the channel parameters to the OVR texture definitions.

    if (setup == false)
    {
        setup = true;

        for (int i = 0; i < 2; i++)
        {
            ovrGLTexture *p = reinterpret_cast<ovrGLTexture*>(tex + i);
            ovrSizei      size;

            size.w = chanv[i]->get_width();
            size.h = chanv[i]->get_height();

            p->OGL.Header.API                 = ovrRenderAPI_OpenGL;
            p->OGL.Header.TextureSize         = size;
            p->OGL.Header.RenderViewport.Size = size;
            p->OGL.TexId                      = chanv[i]->get_color();
        }
    }
    frust[0]->set_eye(vec3(0, 0, 0));
    frust[1]->set_eye(vec3(0, 0, 0));

    dismiss_warning();
}

void dpy::oculus::draw(int chanc, const dpy::channel * const *chanv, int frusi)
{
    ovrHmd_BeginFrame(hmd, 0);
    {
        ovrHmd_GetEyePoses(hmd, 0, offset, pose, NULL);

#if 0
        for (int i = 0; i < 2; i++)
        {
            // Get the head orientation matrix.

            OVR::Quatf q = OVR::Quatf(pose[i].Orientation);
            mat4 O = getMatrix4f(OVR::Matrix4f(q.Inverted()));

            // Get the head offset matrix.

            mat4 T = translation(vec3(-pose[i].Position.x,
                                      -pose[i].Position.y,
                                      -pose[i].Position.z));

            // Set the view matrix.

            ::view->set_tracking(O * T);

            // Draw the scene to the current channel.

            chanv[i]->bind();
            {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                ::host->draw(frusi + i, frust[i], i);
            }
            chanv[i]->free();
        }
#endif
        chanv[0]->bind();
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ::host->draw(frusi + 0, frust[0], 0);
        }
        chanv[0]->free();
        chanv[1]->bind();
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ::host->draw(frusi + 1, frust[1], 1);
        }
        chanv[1]->free();
    }
    ovrHmd_EndFrame(hmd, pose, tex);
    ::host->set_swap();
}

void dpy::oculus::test(int chanc, const dpy::channel *const *chanv, int index)
{
}

//-----------------------------------------------------------------------------

bool dpy::oculus::pointer_to_3D(app::event *E, int x, int y)
{
    // Let the frustum project the pointer into space.

    double s = double(x - viewport[0]) / viewport[2];
    double t = double(y - viewport[1]) / viewport[3];

    if (frust[0]) // HACK 0.0 <= s && s < 1.0 && 0.0 <= t && t < 1.0)
        return frust[0]->pointer_to_3D(E, s, t);
    else
        return false;
}

bool dpy::oculus::process_start(app::event *E)
{
    // Configure the renderer. Zeroing the configuration stucture causes all
    // display, window, and device specifications to take on current values
    // as put in place by SDL. This should work cross-platform (but doesn't).
    // A workaround is currently (0.4.3b) required under linux.

    ovrGLConfig      cfg;
    ovrEyeRenderDesc erd[2];

    memset(&cfg, 0, sizeof (ovrGLConfig));

    cfg.OGL.Header.API      = ovrRenderAPI_OpenGL;
    cfg.OGL.Header.RTSize.w = hmd->Resolution.w;
    cfg.OGL.Header.RTSize.h = hmd->Resolution.h;

    // Set the configuration and receive eye render descriptors in return.

    ovrHmd_ConfigureRendering(hmd, &cfg.Config, ovrDistortionCap_Chromatic
                                                | ovrDistortionCap_TimeWarp
                                                | ovrDistortionCap_Overdrive,
                                                hmd->DefaultEyeFov, erd);

    offset[0] = erd[0].HmdToEyeViewOffset;
    offset[1] = erd[1].HmdToEyeViewOffset;

    // Configure the projections.

    ovrMatrix4f P0 = ovrMatrix4f_Projection(erd[0].Fov, 1.f, 10.f, true);
    ovrMatrix4f P1 = ovrMatrix4f_Projection(erd[1].Fov, 1.f, 10.f, true);

    frust[0] = new app::perspective_frustum(getMatrix4f(P0));
    frust[1] = new app::perspective_frustum(getMatrix4f(P1));

    return false;
}

bool dpy::oculus::process_close(app::event *E)
{
    if (frust[0]) delete frust[0];
    if (frust[1]) delete frust[1];

    frust[0] = 0;
    frust[1] = 0;

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

//------------------------------------------------------------------------------

/// Check if the OVR health & safety warning is visible and try to dismiss it.

void dpy::oculus::dismiss_warning()
{
    if (hmd)
    {
        ovrHSWDisplayState state;

        ovrHmd_GetHSWDisplayState(hmd, &state);

        if (state.Displayed)
            ovrHmd_DismissHSWDisplay(hmd);
    }
}

//-----------------------------------------------------------------------------

#endif
