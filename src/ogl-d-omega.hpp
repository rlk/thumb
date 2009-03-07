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

#ifndef OGL_D_OMEGA_HPP
#define OGL_D_OMEGA_HPP

#include "ogl-process.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class frame;
    class program;
}

//-----------------------------------------------------------------------------

namespace ogl
{
    class d_omega : public process
    {
        const ogl::program *prog;
              ogl::frame   *cube;

    public:

        d_omega(const std::string&);
       ~d_omega();

        void bind(GLenum) const;
        void init();
    };
}

//-----------------------------------------------------------------------------

#endif
