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
    : display(p), hmd(0)
{
    frust[0] = new app::perspective_frustum();
    frust[1] = new app::perspective_frustum();

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

        viewport[0]    = 0;
        viewport[1]    = 0;
        viewport[2]    = hmd->Resolution.w;
        viewport[3]    = hmd->Resolution.h;

        // Determine the buffer size required by each eye of the current HMD.

        ovrSizei sz0 = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left,
                                                hmd->DefaultEyeFov[0], 1);
        ovrSizei sz1 = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right,
                                                hmd->DefaultEyeFov[1], 1);

        buffer_size[0] = std::max(sz0.w, sz1.w);
        buffer_size[1] = std::max(sz0.h, sz1.h);

        // Configure the offscreen render targets.

        memset(tex, 0, 2 * sizeof (ovrTexture));

        for (int i = 0; i < 2; i++)
        {
            ovrGLTexture *p = reinterpret_cast<ovrGLTexture*>(tex + i);

            p->OGL.Header.API                   = ovrRenderAPI_OpenGL;
            p->OGL.Header.TextureSize.w         = buffer_size[0];
            p->OGL.Header.TextureSize.h         = buffer_size[1];
            p->OGL.Header.RenderViewport.Size.w = buffer_size[0];
            p->OGL.Header.RenderViewport.Size.h = buffer_size[1];
        }
    }
}

dpy::oculus::~oculus()
{
    delete frust[1];
    delete frust[0];

    if (hmd) ovrHmd_Destroy(hmd);

    ovr_Shutdown();
}

//-----------------------------------------------------------------------------

int dpy::oculus::get_frusc() const
{
    return 2;
}

void dpy::oculus::get_frusv(app::frustum **frusv) const
{
    assert(frust[0]);
    assert(frust[1]);

    frusv[0] = frust[0];
    frusv[1] = frust[1];
}

//-----------------------------------------------------------------------------

void dpy::oculus::prep(int chanc, const dpy::channel *const *chanv)
{
    // Set the perspective projections.

    ovrHmd_GetEyePoses(hmd, 0, offset, pose, NULL);

    for (int i = 0; i < 2; i++)
    {
        vec3 p = vec3(pose[i].Position.x,
                      pose[i].Position.y,
                      pose[i].Position.z);

        OVR::Quatf q = OVR::Quatf(pose[i].Orientation);

        mat4 O = getMatrix4f(OVR::Matrix4f(q.Inverted()));
        mat4 T = translation(-p);

        frust[i]->set_proj(projection[i] * O * T);
    }

    // Configure OVR to render to the given channels.

    reinterpret_cast<ovrGLTexture*>(tex + 0)->OGL.TexId = chanv[0]->get_color();
    reinterpret_cast<ovrGLTexture*>(tex + 1)->OGL.TexId = chanv[1]->get_color();

    dismiss_warning();
}

void dpy::oculus::draw(int chanc, const dpy::channel * const *chanv, int frusi)
{
    ovrHmd_BeginFrame(hmd, 0);
    {
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

// Configure the renderer. Zeroing the configuration stucture causes all
// display, window, and device specifications to take on current values
// as put in place by SDL. This *should* work cross-platform.

static void configure_renderer(ovrHmd hmd, ovrGLConfig *cfg)
{
    memset(cfg, 0, sizeof (ovrGLConfig));

#if OVR_MAJOR_VERSIEN == 0 && OVR_MINOR_VERSION == 4 && OVR_BUILD_VERSION < 4
    cfg->OGL.Header.API              = ovrRenderAPI_OpenGL;
    cfg->OGL.Header.RTSize.w         = hmd->Resolution.w;
    cfg->OGL.Header.RTSize.h         = hmd->Resolution.h;
#else
    cfg->OGL.Header.API              = ovrRenderAPI_OpenGL;
    cfg->OGL.Header.BackBufferSize.w = hmd->Resolution.w;
    cfg->OGL.Header.BackBufferSize.h = hmd->Resolution.h;
#endif
}

//-----------------------------------------------------------------------------

bool dpy::oculus::pointer_to_3D(app::event *E, int x, int y)
{
    const app::frustum *overlay = ::host->get_overlay();

    // Let the frustum project the pointer into space.

    double s = double(x - viewport[0]) / viewport[2];
    double t = double(y - viewport[1]) / viewport[3];

    if (overlay)
        return overlay->pointer_to_3D(E, s, t);
    else
        return false;
}

bool dpy::oculus::process_start(app::event *E)
{
    ovrGLConfig      cfg;
    ovrEyeRenderDesc erd[2];

    // Set the configuration and receive eye render descriptors in return.

    configure_renderer(hmd, &cfg);
 
    ovrHmd_ConfigureRendering(hmd, &cfg.Config, ovrDistortionCap_Chromatic
                                                | ovrDistortionCap_TimeWarp
                                                | ovrDistortionCap_Overdrive,
                                                hmd->DefaultEyeFov, erd);

    offset[0] = erd[0].HmdToEyeViewOffset;
    offset[1] = erd[1].HmdToEyeViewOffset;

    // Configure the projections.

    projection[0] = getMatrix4f(ovrMatrix4f_Projection(erd[0].Fov, 1.f, 10.f, true));
    projection[1] = getMatrix4f(ovrMatrix4f_Projection(erd[1].Fov, 1.f, 10.f, true));

    return false;
}

bool dpy::oculus::process_close(app::event *E)
{
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
