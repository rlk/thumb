//  Copyright (C) 2009 Robert Kooima
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

#ifndef OGL_IRRADIANCE_ENV_HPP
#define OGL_IRRADIANCE_ENV_HPP

#include <vector>

#include "ogl-process.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class frame;
    class program;
    class process;
    class binding;
}

//-----------------------------------------------------------------------------

namespace ogl
{
    class irradiance_env : public process
    {
        int b;
        int n;
        int m;

        std::vector<const ogl::process *> Y;

        const ogl::process *d;
        const ogl::process *L;

        const ogl::program *init;
        const ogl::program *step;
        const ogl::program *calc;

        ogl::frame *bufA;
        ogl::frame *bufB;
        ogl::frame *cube;

    public:

        irradiance_env(const std::string&);
       ~irradiance_env();

        void draw(const ogl::binding *) const;
        void bind(GLenum)               const;
    };
}

//-----------------------------------------------------------------------------

#endif
