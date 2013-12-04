//  Copyright (C) 2013 Robert Kooima
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

#ifndef DPY_OCULUS_HPP
#define DPY_OCULUS_HPP

#include <OVR.h>

#include <dpy-display.hpp>
#include <app-file.hpp>

//-----------------------------------------------------------------------------

namespace ogl
{
    class program;
}

//-----------------------------------------------------------------------------

namespace dpy
{
    class oculus : public display
    {
    public:

        oculus(app::node);

        virtual ~oculus();

        // Frustum queries

        virtual int  get_frusc()                const;
        virtual void get_frusv(app::frustum **) const;

        virtual app::frustum *get_overlay() const { return frust; }

        // Rendering handlers

        virtual void prep(int, const dpy::channel * const *);
        virtual void draw(int, const dpy::channel * const *, int);
        virtual void test(int, const dpy::channel * const *, int);

        // Event handers

        virtual bool pointer_to_3D(app::event *, int, int);
        virtual bool process_event(app::event *);

    private:

        static OVR::Ptr<OVR::DeviceManager> pManager;
        static OVR::Ptr<OVR::HMDDevice>     pHMD;
        static OVR::Ptr<OVR::SensorDevice>  pSensor;

        static OVR::HMDInfo      Info;
        static OVR::SensorFusion Fusion;

        static OVR::Util::Render::StereoConfig Stereo;

        app::frustum *frust;
        int           chani;

        const ogl::program *program;

        vec2 LensCenter;
        vec4 DistortionK;
        vec4 ChromaAbCorrection;
        vec2 ScaleIn;
        vec2 ScaleOut;

        virtual bool process_start(app::event *);
        virtual bool process_close(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
