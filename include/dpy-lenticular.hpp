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

#ifndef DPY_LENTICULAR_HPP
#define DPY_LENTICULAR_HPP

#include <vector>

#include <etc-vector.hpp>
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
    class lenticular : public display
    {
    public:

        lenticular(app::node);

        virtual ~lenticular();

        // Frustum queries

        virtual int  get_frusc()                const;
        virtual void get_frusv(app::frustum **) const;

        virtual app::frustum *get_overlay() const { return frust[0]; }

        // Rendering handlers

        virtual void prep(int, const dpy::channel * const *);
        virtual void draw(int, const dpy::channel * const *, int);
        virtual void test(int, const dpy::channel * const *, int);

        // Event handers.

        virtual bool pointer_to_3D(app::event *, int, int);
        virtual bool process_event(app::event *);

    private:

        // Modulation waveform configuration

        struct slice_param
        {
            double cycle;
            double step0;
            double step1;
            double step2;
            double step3;
            double depth;

            slice_param(double c) : cycle(c), step0(0), step1(0),
                                    step2(0), step3(0), depth(0) { }
        };

        int channels;

        std::vector<slice_param> slice;

        // Line screen configuration

        double pitch;
        double angle;
        double thick;
        double shift;
        double debug;
        double quality;

        std::vector<app::frustum *> frust;

        const ogl::program *program;

        // Configuration state and event handlers

        app::node array;

        virtual bool process_key  (app::event *);
        virtual bool process_start(app::event *);
        virtual bool process_close(app::event *);

        // Rendering handers

        vec4 calc_transform(const vec3&) const;
        void apply_uniforms()            const;
    };
}

//-----------------------------------------------------------------------------

#endif
