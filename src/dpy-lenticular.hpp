//  Copyright (C) 2007 Robert Kooima
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

#include "dpy-display.hpp"
#include "app-serial.hpp"

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

        double pitch;
        double angle;
        double thick;
        double shift;

        double debug;
        double quality;

        std::vector<app::frustum *> frust;
        std::vector<slice_param>    slice;

        const ogl::program *P;

        virtual bool process_start(app::event *);
        virtual bool process_close(app::event *);

        void calc_transform(const double *, double *) const;
        void apply_uniforms()                         const;

    public:

        lenticular(app::node);

        virtual void get_frustums(std::vector<app::frustum *>&);

        // Rendering handlers.

        virtual void prep(int, dpy::channel **);
        virtual int  draw(int, dpy::channel **,
                          int, app::frustum **);
        virtual int  test(int, dpy::channel **, int);

        // Event handers.

        virtual bool pointer_to_3D(app::event *, int, int);
        virtual bool process_event(app::event *);

        virtual app::frustum *get_overlay() const { return frust[0]; }

        virtual ~lenticular();
    };
}

//-----------------------------------------------------------------------------

#endif
